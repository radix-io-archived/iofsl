#ifndef ZOIDFS_UTIL_ZOIDFSDEFASYNC_HH
#define ZOIDFS_UTIL_ZOIDFSDEFASYNC_HH

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "ZoidFSAPI.hh"
#include "ZoidFSAsync.hh"
#include "iofwdutil/ThreadPool.hh"

#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/Configurable.hh"
#include "iofwdutil/IOFWDLog.hh"

#include "zoidfs/zoidfs-proto.h"
#include "src/zoidfs/util/ZoidFSAsyncWorkUnit.hh"

#include <map>

namespace zoidfs
{

    namespace util
    {

        /* async io error and pending trackers */
        class ZoidFSTrackerData
        {
            public:
                ZoidFSTrackerData() :
                    ref_count_(0), err_(ZFS_OK)
                {
                }

                ZoidFSTrackerData(const ZoidFSTrackerData & rhs)
                {
                    /* lock the rhs */
                    boost::mutex::scoped_lock
                        lock(const_cast<ZoidFSTrackerData &>(rhs).mutex_);

                    /* copy rhs to lhs */
                    ref_count_ = rhs.ref_count_;
                    err_ = rhs.err_;
                }

                const ZoidFSTrackerData & operator= (const ZoidFSTrackerData &
                        rhs)
                {
                    /* check if this is the same obj */
                    if(this == &rhs)
                        return *this;

                    /* lock both objs */
                    boost::mutex::scoped_lock lock1(&mutex_ < &rhs.mutex_ ?
                        mutex_ : const_cast<ZoidFSTrackerData &>(rhs).mutex_);
                    boost::mutex::scoped_lock lock2(&mutex_ >
                        &rhs.mutex_ ? mutex_ : 
                        const_cast<ZoidFSTrackerData &>(rhs).mutex_);

                    /* copy rhs to lhs */
                    ref_count_ = rhs.ref_count_;
                    err_ = rhs.err_;

                    return *this;
                }

                uint64_t ref_count_;
                int err_;
                boost::mutex mutex_;
                boost::condition condition_;
        };

        typedef std::map<zoidfs_handle_t, ZoidFSTrackerData *> ZoidFSHandleTracker;

//==========================================================================

   /**
    * This class implements a non-blocking API on top of a blocking ZoidFS
    * API. Uses a threadpool to call the blocking ZFS calls in the background.
    *
    */
   class ZoidFSDefAsync : public ZoidFSAsync, public iofwdutil::Configurable
   {
   public:

      ZoidFSDefAsync () : 
          log_ (iofwdutil::IOFWDLog::getSource("defasync")),
          tp_(iofwdutil::ThreadPool::instance())
      {
      }

      void configure (const iofwdutil::ConfigFile & config);

      virtual int init(void) ;

      virtual int finalize(void);

      virtual void null(const iofwdevent::CBType & cb, int * ret) ;

      virtual void getattr(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * handle,
              zoidfs_attr_t * attr,
              zoidfs_op_hint_t * op_hint) ;

      virtual void setattr(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * handle,
              const zoidfs_sattr_t * sattr,
              zoidfs_attr_t * attr,
              zoidfs_op_hint_t * hint) ;

      virtual void lookup(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * parent_handle,
              const char * component_name,
              const char * full_path,
              zoidfs_handle_t * handle,
              zoidfs_op_hint_t * hint) ;

      virtual void readlink(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * handle,
              char * buffer,
              size_t buffer_length,
              zoidfs_op_hint_t * hint) ;

      virtual void read(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * handle,
              size_t mem_count,
              void * mem_starts[],
              const size_t mem_sizes[],
              size_t file_count,
              const zoidfs_file_ofs_t file_starts[],
              const zoidfs_file_size_t file_sizes[],
              zoidfs_op_hint_t * hint) ;

      virtual void write(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * handle,
              size_t mem_count,
              const void * mem_starts[],
              const size_t mem_sizes[],
              size_t file_count,
              const zoidfs_file_ofs_t file_starts[],
              const zoidfs_file_size_t file_sizes[],
              zoidfs_op_hint_t * hint) ;

      virtual void commit(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * handle,
              zoidfs_op_hint_t * hint) ;

      virtual void create(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * parent_handle,
              const char * component_name,
              const char * full_path,
              const zoidfs_sattr_t * attr,
              zoidfs_handle_t * handle,
              int * created,
              zoidfs_op_hint_t * hint) ;

      virtual void remove(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * parent_handle,
              const char * component_name,
              const char * full_path,
              zoidfs_cache_hint_t * parent_hint,
              zoidfs_op_hint_t * hint) ;

      virtual void rename(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * from_parent_handle,
              const char * from_component_name,
              const char * from_full_path,
              const zoidfs_handle_t * to_parent_handle,
              const char * to_component_name,
              const char * to_full_path,
              zoidfs_cache_hint_t * from_parent_hint,
              zoidfs_cache_hint_t * to_parent_hint,
              zoidfs_op_hint_t * hint) ;

      virtual void link(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * from_parent_handle,
              const char * from_component_name,
              const char * from_full_path,
              const zoidfs_handle_t * to_parent_handle,
              const char * to_component_name,
              const char * to_full_path,
              zoidfs_cache_hint_t * from_parent_hint,
              zoidfs_cache_hint_t * to_parent_hint,
              zoidfs_op_hint_t * hint) ;

      virtual void symlink(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * from_parent_handle,
              const char * from_component_name,
              const char * from_full_path,
              const zoidfs_handle_t * to_parent_handle,
              const char * to_component_name,
              const char * to_full_path,
              const zoidfs_sattr_t * attr,
              zoidfs_cache_hint_t * from_parent_hint,
              zoidfs_cache_hint_t * to_parent_hint,
              zoidfs_op_hint_t * hint) ;

      virtual void mkdir(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * parent_handle,
              const char * component_name,
              const char * full_path,
              const zoidfs_sattr_t * attr,
              zoidfs_cache_hint_t * parent_hint,
              zoidfs_op_hint_t * hint) ;

      virtual void readdir(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * parent_handle,
              zoidfs_dirent_cookie_t cookie,
              size_t * entry_count,
              zoidfs_dirent_t * entries,
              uint32_t flags,
              zoidfs_cache_hint_t * parent_hint,
              zoidfs_op_hint_t * hint) ;

      virtual void resize(const iofwdevent::CBType & cb,
              int * ret,
              const zoidfs_handle_t * handle,
              zoidfs_file_size_t size,
              zoidfs_op_hint_t * hint) ;

   protected:

      static void runWorkUnit(ZoidFSDefAsyncWorkUnit * wu);

      struct OpHelper
      {
         OpHelper (const iofwdevent::CBType & mcb,
                 int * ret,
                 const boost::function<int ()> & wcb)
             : mcb_(mcb),
             ret_(ret),
             wcb_(wcb)
         {
         }

         void operator () ()
         {
            try
            {
               *ret_ = wcb_ ();
            }
            catch (const iofwdutil::ZException & e)
            {
               mcb_ (iofwdevent::CBException (boost::current_exception ()));
               return;
            }
            catch (...)
            {
               // Nobody should be throwing non-ZException derived exceptions!
               ALWAYS_ASSERT(false && "Should not happen!");
            }
            mcb_ (iofwdevent::CBException ());
         }

         void run()
         {
            this->operator () ();
         }

         iofwdevent::CBType mcb_;
         int * ret_;
         boost::function<int ()> wcb_;
      };

    void submitWorkUnit(const boost::function<void (void)> & wu_,
          iofwdutil::ThreadPool::TPPrio prio)
    {
        if(wait_for_threads_)
        {
            tp_.submitWorkUnit(wu_, prio);
        }
        else
        {
            boost::thread t (wu_);
        }
    }

   protected:
      boost::scoped_ptr<ZoidFSAPI> api_;
      bool wait_for_threads_;

      iofwdutil::zlog::ZLogSource & log_;
      iofwdutil::ThreadPool & tp_;
      bool highpriooplist_[zoidfs::ZOIDFS_PROTO_MAX];

      static boost::mutex handle_tracker_mutex_;
      static ZoidFSHandleTracker handle_tracker_;
   };

//==========================================================================
    } /* namespace util */
} /* namespace zoidfs */

#endif /* __ZOIDFS_UTIL_ZOIDFSDEFASYNC_HH__ */
