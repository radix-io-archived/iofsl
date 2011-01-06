#ifndef ZOIDFS_UTIL_RATELIMITERASYNC_HH
#define ZOIDFS_UTIL_RATELIMITERASYNC_HH

#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/optional.hpp>

#include "ZoidFSAsyncPT.hh"
#include "iofwdutil/Configurable.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwdevent/TokenResource.hh"
#include "iofwdevent/ResourceWrapper.hh"

namespace iofwdevent
{
   class TimerResource;
}

namespace zoidfs
{
   namespace util
   {
      //=====================================================================

      /**
       * This ZoidFSAsync implementation passes all operations except read
       * and write through to another ZoidFSAsync instance.
       * Read and write are delayed according to parameters passed to it in
       * the configure method.
       *
       * It uses the iofwdevent::TimerResource to limit bw.
       *
       * @TODO This really doesn't belong in src/zoidfs/util ...
       *
       */
      class RateLimiterAsync : public ZoidFSAsyncPT,
                                public iofwdutil::Configurable
      {
         public:
            RateLimiterAsync ();

            void configure (const iofwdutil::ConfigFile & config);

            /**
             * Init it used to start the timer.
             */
            virtual int init ();

            virtual int finalize ();

            /**
             * Capture and limit read call.
             */
            virtual void read(const iofwdevent::CBType & cb, int * ret, const
                  zoidfs_handle_t * handle,
                  size_t mem_count,
                  void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const zoidfs_file_ofs_t file_starts[],
                  const zoidfs_file_size_t file_sizes[],
                  zoidfs_op_hint_t * hint);

            /**
             * Capture write calls
             */
            virtual void write(const iofwdevent::CBType & cb, int * ret, const
                  zoidfs_handle_t * handle,
                  size_t mem_count,
                  const void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const zoidfs_file_ofs_t file_starts[],
                  const zoidfs_file_size_t file_sizes[],
                  zoidfs_op_hint_t * hint);


         protected:
            enum { KB = 1024 } ;

            /// Get called every second
            void timerTick (iofwdevent::CBException e);


         protected:
            class DelayedOp
            {
               public:

                  DelayedOp (RateLimiterAsync & parent)
                     : parent_(parent),
                       status_ (INIT),
                       obtained_ (0)
                  {
                  }

                  bool callback (iofwdevent::CBException e);

                  void execOp ();

               protected:
                  friend class RateLimiterAsync;

                  enum { INIT = 0,
                     WAITING_FOR_LAT,
                     WAITING_FOR_PRE_EXEC,
                     WAITING_FOR_TOKEN_1,
                     WAITING_FOR_TOKEN_2,
                     WAITING_FOR_POST_EXEC,
                     DONE
                  };

                  RateLimiterAsync & parent_;
                  iofwdevent::CBType cb_;

                  bool read_;
                  int status_;
                  iofwdevent::CBException op_status_;  // status returned by
                                                       // read/write op
                  bool delay_issue_;
                  size_t transfersize_;
                  size_t obtained_;

                  iofwdevent::CBType usercb;
                  int * ret;
                  const zoidfs_handle_t * handle;
                  size_t mem_count;
                  void * * mem_starts;
                  const size_t * mem_sizes;
                  size_t file_count;
                  const zoidfs_file_ofs_t * file_starts;
                  const zoidfs_file_size_t * file_sizes;
                  zoidfs_op_hint_t * hint;

                  boost::mutex lock_;
            };


            DelayedOp * newRead (const iofwdevent::CBType & cb, int * ret,
                  const zoidfs_handle_t * handle, size_t mem_count, void *
                  mem_starts[], const size_t mem_sizes[], size_t file_count,
                  const zoidfs_file_ofs_t file_starts[], const zoidfs_file_size_t
                  file_sizes[], zoidfs_op_hint_t * hint);

            DelayedOp * newWrite (const iofwdevent::CBType & cb, int * ret,
                  const zoidfs_handle_t * handle, size_t mem_count, const void
                  * mem_starts[], const size_t mem_sizes[], size_t file_count,
                  const zoidfs_file_ofs_t file_starts[], const zoidfs_file_size_t
                  file_sizes[], zoidfs_op_hint_t * hint);

             void scheduleTimer ();

             static void callback (DelayedOp * op, iofwdevent::CBException e);

         protected:
            iofwdutil::IOFWDLogSource & log_;

            iofwdevent::TimerResource & timer_;

            iofwdevent::TokenResource op_tokens_;
            iofwdevent::TokenResource r_bw_tokens_;
            iofwdevent::TokenResource w_bw_tokens_;

            iofwdevent::ResourceWrapper opwrapper_;
            iofwdevent::ResourceWrapper rbwwrapper_;
            iofwdevent::ResourceWrapper wbwwrapper_;

            size_t op_limit_;
            size_t read_bw_;
            size_t write_bw_;
            size_t op_burst_limit_;
            size_t read_burst_bw_;
            size_t write_burst_bw_;
            size_t hz_;
            size_t latency_;

            bool delay_issue_;

            boost::optional<iofwdevent::Handle> timerhandle_;

            typedef boost::recursive_mutex MutexType;
            MutexType lock_;

            bool shutdown_;
      };


      //=====================================================================
   }
}
#endif
