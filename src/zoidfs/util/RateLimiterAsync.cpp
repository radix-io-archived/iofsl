#include <numeric>

#include "RateLimiterAsync.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/Factory.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdevent/TimerResource.hh"
#include "iofwdutil/assert.hh"

using boost::format;


GENERIC_FACTORY_CLIENT(std::string,
      zoidfs::util::ZoidFSAsync,
      zoidfs::util::RateLimiterAsync,
      "ratelimit",
      ratelimit);


namespace zoidfs
{
   namespace util
   {
      //=====================================================================

      RateLimiterAsync::RateLimiterAsync ()
         : log_ (iofwdutil::IOFWDLog::getSource ("ratelimiter")),
         timer_ (iofwdevent::TimerResource::instance ()),
         opwrapper_(&op_tokens_),
         rbwwrapper_(&r_bw_tokens_),
         wbwwrapper_(&w_bw_tokens_),
         shutdown_(false)
      {
      }

      /**
       * Assumes lock is held.
       */
      void RateLimiterAsync::scheduleTimer ()
      {
         timerhandle_ = timer_.createTimer
            (boost::bind(&RateLimiterAsync::timerTick, this, _1), 1000);
      }

      int RateLimiterAsync::init ()
      {
         {
            MutexType::scoped_lock l(lock_);
            shutdown_ = false;
            scheduleTimer ();
         }

         return ZoidFSAsyncPT::init ();
      }

      int RateLimiterAsync::finalize ()
      {
         {
            MutexType::scoped_lock l(lock_);

            shutdown_ = true;

            if (timerhandle_)
            {
               ZLOG_INFO(log_, "Cancelling timer...");
               if (timer_.cancel (*timerhandle_))
               {
                  ZLOG_INFO(log_, "Timer cancelled...");
               }
               else
               {
                  ZLOG_INFO(log_, "Timer cancelled failed (this is not a "
                        "problem)");
               }
               // remove value from optional<Handle>
               timerhandle_ = boost::none;
            }
         }
         return ZoidFSAsyncPT::finalize ();
      }

      void RateLimiterAsync::timerTick (int status)
      {
         if (status == iofwdevent::CANCELLED)
         {
            ZLOG_INFO(log_, "(timer callback) Timer cancelled");
            return;
         }

         MutexType::scoped_lock l(lock_);

         ALWAYS_ASSERT(!shutdown_);

         ZLOG_DEBUG(log_, "Timer Tick...");
         r_bw_tokens_.add_tokens_limit (read_bw_, read_burst_bw_);
         w_bw_tokens_.add_tokens_limit (write_bw_, write_burst_bw_);
         op_tokens_.add_tokens_limit (op_limit_, op_burst_limit_);
         scheduleTimer ();
      }

      void RateLimiterAsync::configure (const iofwdutil::ConfigFile & config)
      {
         const std::string api (config.getKeyDefault ("api", "zoidfs"));
         read_bw_ = config.getKeyAsDefault ("read_bw", 64*1024);
         write_bw_ = config.getKeyAsDefault ("write_bw", 64*1024);
         op_limit_ = config.getKeyAsDefault ("op_limit", 1024);

         read_burst_bw_ = config.getKeyAsDefault ("read_burst_bw_", read_bw_);
         write_burst_bw_ = config.getKeyAsDefault ("write_burst_bw", write_bw_);
         op_burst_limit_ = config.getKeyAsDefault ("op_burst_limit", op_limit_);

         // Config file values are in KB/s -> go to bytes
         read_burst_bw_ *= KB;
         write_burst_bw_ *= KB;
         read_bw_ *= KB;
         write_bw_ *= KB;

         delay_issue_ = config.getKeyAsDefault ("delay_issue", false);

         ZLOG_INFO (log_, format("Read bw %i kb/s, write bw %i kb/s, op limit"
                  "%i ops/s") % read_bw_ % write_bw_ % op_limit_);
         ZLOG_INFO (log_, format("Burst read bw %i kb/s, burst write bw %i kb/s,"
                  " burst op limit %i ops/s") % read_burst_bw_ %
               write_burst_bw_ % op_burst_limit_);
         ZLOG_INFO (log_, (delay_issue_ ? "Delaying issuing of requests..."
                  : "Requests will be issued immediately;"
                  " Reponses will be delayed."));

         ZLOG_INFO (log_, format("Using api '%s'") % api);
         api_.reset (iofwdutil::Factory<
               std::string,
               zoidfs::util::ZoidFSAsync>::construct(api)());
         iofwdutil::Configurable::configure_if_needed (api_.get(),
               config.openSectionDefault(api.c_str()));

         // For ZoidFSAsyncPT
         setAsyncPT (api_.get());
      }

      /**
       * This method is here to ensure we don't delete a DelayedOp instance
       * from within itself. (although it might still happen ;-)
       */
      void RateLimiterAsync::callback (DelayedOp * op, int status)
      {
         MutexType::scoped_lock l(op->parent_.lock_);

         if (!op->callback (status))
            delete op;
      }

      void RateLimiterAsync::DelayedOp::execOp ()
      {
         ZLOG_DEBUG(parent_.log_, format("(op %p) posting read/write")%this);
         if (read_)
         {
            parent_.ZoidFSAsyncPT::read (cb_, ret, handle, mem_count,
                  const_cast<void **>(mem_starts), mem_sizes,
                  file_count, file_starts, file_sizes, hint);
         }
         else
         {
            parent_.ZoidFSAsyncPT::write (cb_, ret, handle, mem_count,
                  const_cast<const void **>(mem_starts), mem_sizes,
                  file_count, file_starts, file_sizes,
                  hint);
         }
      }

      /**
       * If it returns false, it gets deleted.
       */
      bool RateLimiterAsync::DelayedOp::callback (int
            cbstat)
      {
         // Cannot use a simple lock here since the callback could be called
         // recursively from for example TokenResource.request.
         // If locking is needed, it needs to be a recursive lock

         switch (status_)
         {
            case INIT:
               ZLOG_DEBUG_MORE(parent_.log_, format("(op %p): in init")%this);
               transfersize_ = std::accumulate (&mem_sizes[0],
                     &mem_sizes[mem_count], 0);
               status_ = WAITING_FOR_PRE_EXEC;
               if (!delay_issue_)
               {
                  execOp ();
                  return true;
               }
               // if we delay issuing exec fall through to getting tokens
            case WAITING_FOR_PRE_EXEC:
               ZLOG_DEBUG_MORE(parent_.log_, format("(op %p): in pre_exec")%this);
               op_status_ = cbstat;
               status_ = WAITING_FOR_TOKEN_1;
               // get tokens for operation
               // Note that the cb could fire immediately (before returning
               // from request) so we need to update the status_ first so we
               // don't end up here again.
               parent_.op_tokens_.request (cb_, 1);
               return true;
            case WAITING_FOR_TOKEN_1:
               ZLOG_DEBUG_MORE(parent_.log_, format("(op %p): in waiting_for_op_token")%this);
               status_ = WAITING_FOR_TOKEN_2;
               // get tokens for bw
               if (read_)
                  parent_.r_bw_tokens_.request (cb_, transfersize_);
               else
                  parent_.w_bw_tokens_.request (cb_, transfersize_);
               return true;
            case WAITING_FOR_TOKEN_2:
               ZLOG_DEBUG_MORE(parent_.log_, format("(op %p): in waiting_for_bwtoken")%this);
               status_ = WAITING_FOR_POST_EXEC;
               if (delay_issue_)
               {
                  execOp ();
                  return true;
               }
            case WAITING_FOR_POST_EXEC:
               ZLOG_DEBUG_MORE(parent_.log_, format("(op %p): in waiting_for_postexec")%this);
               if (delay_issue_)
               {
                  op_status_ = cbstat;
               }
            case DONE:
               // all done: call real callback
               ZLOG_DEBUG_MORE(parent_.log_, format("(op %p): all done;"
                        " calling user callback")%this);
               usercb (op_status_);
               return false;
         }
         return true;
      }

      RateLimiterAsync::DelayedOp * RateLimiterAsync::newRead (const
            iofwdevent::CBType & cb, int * ret, const
            zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         DelayedOp * o = new DelayedOp (*this);
         o->cb_ = boost::bind (&RateLimiterAsync::callback, o, _1);
         o->delay_issue_ = delay_issue_;
         o->read_ = true;

         o->usercb = cb;
         o->ret = ret;
         o->handle = handle;
         o->mem_count = mem_count;
         o->mem_starts = mem_starts;
         o->mem_sizes = mem_sizes;
         o->file_count = file_count;
         o->file_starts = file_starts;
         o->file_sizes = file_sizes;
         o->hint = hint;
         return o;
      }

      RateLimiterAsync::DelayedOp * RateLimiterAsync::newWrite (const
            iofwdevent::CBType & cb, int
            * ret, const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         DelayedOp * o = new DelayedOp (*this);
         o->cb_ = boost::bind(&RateLimiterAsync::callback, o, _1);
         o->delay_issue_ = delay_issue_;
         o->read_ = false;

         o->usercb = cb;
         o->ret = ret;
         o->handle = handle;
         o->mem_count = mem_count;
         o->mem_starts = const_cast<void**>(mem_starts);
         o->mem_sizes = mem_sizes;
         o->file_count = file_count;
         o->file_starts = file_starts;
         o->file_sizes = file_sizes;
         o->hint = hint;
         return o;
      }

      void RateLimiterAsync::read(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         DelayedOp * op = newRead (cb, ret, handle, mem_count,
               mem_starts, mem_sizes, file_count, file_starts, file_sizes,
               hint);

         callback (op, iofwdevent::COMPLETED);
      }

      void RateLimiterAsync::write(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         DelayedOp * op = newWrite (cb, ret, handle, mem_count,
               mem_starts, mem_sizes, file_count, file_starts, file_sizes,
              hint);

         callback (op, iofwdevent::COMPLETED);
      }


      //=====================================================================
   }
}
