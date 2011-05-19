#include <memory>
#include <vector>
#include <string>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"
#include "ZoidFSDefAsync.hh"
#include "iofwdutil/ConfigFile.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

#include "zoidfs/hints/zoidfs-hints.h"

#include <cassert>

#include "iofwdutil/mm/NBIOMemoryManager.hh"

#include <boost/thread/thread_time.hpp>

using boost::format;

GENERIC_FACTORY_CLIENT(std::string,
      zoidfs::util::ZoidFSAsync,
      zoidfs::util::ZoidFSDefAsync,
      "defasync",
      defasync);

namespace zoidfs
{
    namespace util
    {

   //=====================================================================
        boost::mutex ZoidFSDefAsync::handle_tracker_mutex_;
        boost::mutex ZoidFSDefAsync::commit_mutex_;
        ZoidFSHandleTracker ZoidFSDefAsync::handle_tracker_;
        uint64_t ZoidFSDefAsync::handle_tracker_id_ = 0;

   ZoidFSDefAsync::~ZoidFSDefAsync()
   {
        ZoidFSHandleTracker::iterator it;
        boost::mutex::scoped_lock lock(handle_tracker_mutex_);

        for(it = handle_tracker_.begin() ; it != handle_tracker_.end() ; it++)
        {
            delete it->second;
        }

        handle_tracker_.clear();
    }

   void ZoidFSDefAsync::runWorkUnit(ZoidFSDefAsyncWorkUnit * bwu)
   {
       switch(bwu->type_)
       {
           case zoidfs::ZOIDFS_PROTO_NULL:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncNullWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncNullWorkUnit *>(bwu);

              boost::exception_ptr e;
              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->null();
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(iofwdevent::CBException (e));

              break;
           }
           case zoidfs::ZOIDFS_PROTO_GET_ATTR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncGetattrWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncGetattrWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->getattr(wu->handle_, wu->attr_,
                       wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_SET_ATTR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncSetattrWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncSetattrWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->setattr(wu->handle_, wu->sattr_,
                       wu->attr_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_LOOKUP:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncLookupWorkUnit * wu =
                  static_cast<ZoidFSDefAsyncLookupWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->lookup(wu->parent_handle_,
                       wu->component_name_, wu->full_path_, wu->handle_,
                       wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_READLINK:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncReadlinkWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncReadlinkWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->readlink(wu->handle_, wu->buffer_,
                       wu->buffer_length_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_COMMIT:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncCommitWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncCommitWorkUnit *>(bwu);

              boost::exception_ptr e;

            boost::mutex::scoped_try_lock mlock(commit_mutex_);
            try
            {
                int nbio_flag = 0;
                int dropio_flag = 0;
                int dropio_vl = 0;
                int nbio_vl = 0;

                /* look for the non-block io hint */ 
                if(wu->hint_)
                {
                    zoidfs::hints::zoidfs_hint_get_valuelen(*(wu->hint_),
                        ZOIDFS_NONBLOCK_SERVER_IO, &nbio_vl, &nbio_flag);
                    zoidfs::hints::zoidfs_hint_get_valuelen(*(wu->hint_),
                        ZOIDFS_NONBLOCK_SERVER_DROP_IO, &dropio_vl, &dropio_flag);
                }

                if(dropio_flag)
                {
                    *(wu->ret_) = ZFS_OK;
                }
                else
                {
                    /* invoke the write API */
                    *(wu->ret_) = wu->api_->commit(wu->handle_, wu->hint_);
                }
          
                /* check for completed async io if the hint was set */
                if(nbio_flag) 
                {
                    ZoidFSTrackerData * data = NULL;
                    {
                        ZoidFSHandleTracker::iterator it;
                        boost::mutex::scoped_lock lock(handle_tracker_mutex_);

                        /* if the handle is in the map, get it */
                        if((it = handle_tracker_.find(ZoidFSTrackerKey(wu->handle_))) != 
                                handle_tracker_.end())
                        {
                            data = it->second;
                        }
                    }

                    /* if the handle was in the tracker table */
                    if(data)
                    {
                        uint64_t ref_count = 0;
                        uint64_t ref_count_orig = 0;
                        boost::posix_time::time_duration duration =
                            boost::posix_time::milliseconds(10);

                        {
                            boost::mutex::scoped_lock lock(data->mutex_);

                            data->flush_ = true;
                            ref_count = data->ref_count_;
                            ref_count_orig = data->ref_count_;

                            while(ref_count > 0)
                            {
                                data->condition_.timed_wait(lock,
                                        boost::get_system_time() + duration);
                                ref_count = data->ref_count_;
                            }
                        }

                        if(data->err_ != ZFS_OK)
                        {
                            *(wu->ret_) = data->err_;
                            data->err_ = ZFS_OK;
                        }
                    }
                 }
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_CREATE:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncCreateWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncCreateWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->create(wu->parent_handle_,
                       wu->component_name_, wu->full_path_, wu->sattr_,
                       wu->handle_, wu->created_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_REMOVE:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncRemoveWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncRemoveWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->remove(wu->parent_handle_,
                       wu->component_name_, wu->full_path_, wu->parent_hint_,
                       wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_RENAME:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncRenameWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncRenameWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->rename(wu->from_parent_handle_,
                       wu->from_component_name_, wu->from_full_path_,
                       wu->to_parent_handle_, wu->to_component_name_,
                       wu->to_full_path_, wu->from_parent_hint_,
                       wu->to_parent_hint_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_LINK:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncLinkWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncLinkWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->link(wu->from_parent_handle_,
                       wu->from_component_name_, wu->from_full_path_,
                       wu->to_parent_handle_, wu->to_component_name_,
                       wu->to_full_path_,
                       wu->from_parent_hint_, wu->to_parent_hint_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_SYMLINK:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncSymlinkWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncSymlinkWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->symlink(wu->from_parent_handle_,
                       wu->from_component_name_, wu->from_full_path_,
                       wu->to_parent_handle_, wu->to_component_name_,
                       wu->to_full_path_,
                       wu->sattr_, wu->from_parent_hint_, wu->to_parent_hint_,
                       wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_MKDIR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncMkdirWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncMkdirWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->mkdir(wu->parent_handle_,
                       wu->component_name_, wu->full_path_, wu->sattr_,
                       wu->parent_hint_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_READDIR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncReaddirWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncReaddirWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->readdir(wu->parent_handle_,
                       wu->cookie_, wu->entry_count_, wu->entries_,
                       wu->flags_, wu->parent_hint_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_RESIZE:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncResizeWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncResizeWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 *(wu->ret_) = wu->api_->resize(wu->handle_, wu->size_,
                       wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_READ:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncReadWorkUnit * wu =
                 static_cast<ZoidFSDefAsyncReadWorkUnit *>(bwu);

              boost::exception_ptr e;

              try
              {
                 /* invoke the write API */
                 *(wu->ret_) = wu->api_->read(wu->handle_, wu->mem_count_,
                       wu->mem_starts_, wu->mem_sizes_,
                       wu->file_count_, wu->file_starts_,
                       wu->file_sizes_, wu->hint_);
              }
              catch (...)
              {
                 e = boost::current_exception ();
              }

              /* invoke the callback */
              wu->cb_(e);

              break;
           }

           case zoidfs::ZOIDFS_PROTO_WRITE:
           {
                /* convert to the write work unit */
                ZoidFSDefAsyncWriteWorkUnit * wu =
                    static_cast<ZoidFSDefAsyncWriteWorkUnit *>(bwu);

                boost::exception_ptr e;
                unsigned int i = 0;
                bool issue_cb = true;

                try
                {
                    int dropnbio_flag = 0;
                    int dropaaio_flag = 0;

                    if(wu->hint_)
                    {
                        int vl = 0;
                        zoidfs::hints::zoidfs_hint_get_valuelen(*(wu->hint_),
                            ZOIDFS_NONBLOCK_SERVER_DROP_IO, &vl, &dropnbio_flag);
                        zoidfs::hints::zoidfs_hint_get_valuelen(*(wu->hint_),
                            ZOIDFS_ATOMIC_APPEND_DROP_IO, &vl, &dropaaio_flag);
                    }

                    if(dropnbio_flag || dropaaio_flag)
                    {
                        *(wu->ret_) = ZFS_OK;
                    }
                    else
                    {
                        *(wu->ret_) = wu->api_->write(wu->handle_, wu->mem_count_,
                            wu->mem_starts_, wu->mem_sizes_, wu->file_count_,
                            wu->file_starts_, wu->file_sizes_, wu->hint_);
                    }

                    if(wu->op_key_)
                    {
                        issue_cb = false;

                        /* free mem allocated for this request */
                        for(i = 0 ; i < wu->mem_count_ ; i++)
                        {
                            delete [] static_cast<char *>(const_cast<void *>
                                (wu->mem_starts_[i])); 
                        }
                        delete [] wu->mem_starts_;
                        delete [] wu->mem_sizes_;
                        delete [] wu->file_starts_; 
                        delete [] wu->file_sizes_;

                        /* cleanup mem alloc */
                        iofwdutil::mm::NBIOMemoryManager::instance().dealloc(wu->alloc_); 
                        delete wu->alloc_;

                        if(wu->hint_)
                        {
                            zoidfs::hints::zoidfs_hint_free(wu->hint_);
                            delete wu->hint_;
                        }
        
                        {
                            boost::mutex::scoped_lock
                                lock(ZoidFSDefAsync::handle_tracker_mutex_);

                            delete wu->op_key_;
                        }
                    }

                    {
                        ZoidFSTrackerData * data = NULL;
                        {
                            boost::mutex::scoped_lock 
                                handle_lock(handle_tracker_mutex_);
                            ZoidFSHandleTracker::iterator it;

                           if((it = handle_tracker_.find(ZoidFSTrackerKey(wu->handle_)))
                                   != handle_tracker_.end())
                           {
                               data = it->second;
                           }
                           if(!issue_cb)
                           {
                               delete wu->handle_;
                           }
                        }
                        if(data && !issue_cb)
                        {
                            {
                                boost::mutex::scoped_lock 
                                    handle_lock(data->mutex_); 
                    
                                /* dec the handle ref count */
                                assert(data->ref_count_ > 0);
                                data->ref_count_--;

                                /* check the error code and update the error tracker if
                                a failure was detected */
                                if(*(wu->ret_) != ZFS_OK)
                                {
                                    data->err_ = *(wu->ret_);
                                }
                                delete wu->ret_;

                                if(data->flush_)
                                {
                                    data->condition_.notify_one();
                                }
                            }
                        }
                    }
                }
                catch (...)
                {
                    e = boost::current_exception ();
                }

                /* invoke the callback */
                if(issue_cb)
                {
                    wu->cb_(e);
                }

                break;
           }
           default:
           {
              ALWAYS_ASSERT(false);
              break;
           }
       };

       delete bwu;
   }

   int ZoidFSDefAsync::init(void)
   {
      return api_->init ();
   }

   int ZoidFSDefAsync::finalize(void)
   {
      return api_->finalize ();
   }

   void ZoidFSDefAsync::configure (const iofwdutil::ConfigFile & config)
   {
      // Try to get the name of the blocking zoidfs API
      std::string apiname = config.getKeyDefault ("blocking_api", "zoidfs");
      ZLOG_INFO (log_, format("Using blocking API '%s'") % apiname);

      // @TODO: get blocking API through factory here
      try
      {
         api_.reset (iofwdutil::Factory<
               std::string,
               zoidfs::util::ZoidFSAPI>::construct (apiname)());
         // Configure blocking api_
         Configurable::configure_if_needed (api_.get(),
               config.openSectionDefault(apiname.c_str()));
      }
      catch (iofwdutil::FactoryException & e)
      {
         ZLOG_ERROR(log_, format("Could not instantiate blocking API '%s'!") %
               apiname);
         // TODO: translate exception?
         throw;
      }

      /* check for wait for threads param */
      try
      {
         wait_for_threads_ = config.getKeyAsDefault<bool>("use_thread_pool", false);
         if (wait_for_threads_)
         {
            ZLOG_INFO(log_, "Using thread pool...");
         }
         else
         {
            ZLOG_INFO(log_, "Not using thread pool...");
         }
      }
      catch (const boost::bad_lexical_cast &)
      {
         ZLOG_ERROR(log_, "Invalid value for use_thread_pool: use '0' or"
               " '1'");
         throw;
      }

      /* check for high prio operations... defaults to READ and WRITE */
      try
      {
         std::vector<std::string> op_strs;
         std::string highprioops_ = config.getKeyAsDefault<std::string>("highprioops", "READ,WRITE");
         boost::split(op_strs, highprioops_, boost::is_any_of(","));

         for(int i = 0 ; i < zoidfs::ZOIDFS_PROTO_MAX ; i++)
         {
             highpriooplist_[i] = false;
         }

         for(unsigned int i = 0 ; i < op_strs.size() ; i++)
         {
            ZLOG_INFO(log_, format("'%s' is a high priority operation") %
               op_strs[i]);

             if(op_strs[i].compare("READ") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_READ] = true;
             else if(op_strs[i].compare("WRITE") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_WRITE] = true;
             else if(op_strs[i].compare("GET_ATTR") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_GET_ATTR] = true;
             else if(op_strs[i].compare("SET_ATTR") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_SET_ATTR] = true;
             else if(op_strs[i].compare("REMOVE") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_REMOVE] = true;
             else if(op_strs[i].compare("CREATE") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_CREATE] = true;
             else if(op_strs[i].compare("MKDIR") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_MKDIR] = true;
             else if(op_strs[i].compare("LOOKUP") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_LOOKUP] = true;
             else if(op_strs[i].compare("READLINK") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_READLINK] = true;
             else if(op_strs[i].compare("COMMIT") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_COMMIT] = true;
             else if(op_strs[i].compare("RENAME") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_RENAME] = true;
             else if(op_strs[i].compare("SYMLINK") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_SYMLINK] = true;
             else if(op_strs[i].compare("READDIR") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_READDIR] = true;
             else if(op_strs[i].compare("RESIZE") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_RESIZE] = true;
             else if(op_strs[i].compare("LINK") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_LINK] = true;
             else if(op_strs[i].compare("NULL") == 0)
                highpriooplist_[zoidfs::ZOIDFS_PROTO_NULL] = true;
         }
      }
      catch (const boost::bad_lexical_cast &)
      {
         ZLOG_ERROR(log_, "Invalid value for use_thread_pool: use '0' or"
               " '1'");
         throw;
      }

   }


   void ZoidFSDefAsync::null(const iofwdevent::CBType & cb, int * ret)
   {
      ZoidFSDefAsyncNullWorkUnit * wu = new ZoidFSDefAsyncNullWorkUnit(cb,
              ret, api_.get(), tp_);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_NULL])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::getattr(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * handle,
           zoidfs_attr_t * attr,
           zoidfs_op_hint_t * op_hint)
   {
      ZoidFSDefAsyncGetattrWorkUnit * wu = new ZoidFSDefAsyncGetattrWorkUnit(cb,
              ret, api_.get(), tp_, handle, attr, op_hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_GET_ATTR])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::setattr(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * handle,
           const zoidfs_sattr_t * sattr,
           zoidfs_attr_t * attr,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncSetattrWorkUnit * wu = new ZoidFSDefAsyncSetattrWorkUnit(cb,
              ret, api_.get(), tp_, handle, sattr, attr, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_SET_ATTR])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::lookup(const iofwdevent::CBType & cb, int * ret,
         const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         zoidfs_handle_t * handle,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncLookupWorkUnit * wu = new ZoidFSDefAsyncLookupWorkUnit(cb,
              ret, api_.get(), tp_, parent_handle, component_name,
              full_path, handle, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_LOOKUP])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::readlink(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * handle,
           char * buffer,
           size_t buffer_length,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncReadlinkWorkUnit * wu = new ZoidFSDefAsyncReadlinkWorkUnit(cb,
              ret, api_.get(), tp_, handle, buffer, buffer_length, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_READLINK])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::read(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * handle,
           size_t mem_count,
           void * mem_starts[],
           const size_t mem_sizes[],
           size_t file_count,
           const zoidfs_file_ofs_t file_starts[],
           const zoidfs_file_size_t file_sizes[],
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncReadWorkUnit * wu = new ZoidFSDefAsyncReadWorkUnit(cb,
              ret, api_.get(), tp_, handle, mem_count, mem_starts,
              mem_sizes, file_count, file_starts, file_sizes, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_READ])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::write(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * handle,
           size_t mem_count,
           const void * mem_starts[],
           const size_t mem_sizes[],
           size_t file_count,
           const zoidfs_file_ofs_t file_starts[],
           const zoidfs_file_size_t file_sizes[],
           zoidfs_op_hint_t * hint)
   {
        /* atomic append hint vars */
        int nbvallen = 0;
        int nbflag = 0;

        /* check if the atomic append hint was set */
        if(hint)
        {
            zoidfs::hints::zoidfs_hint_get_valuelen(*hint,
                ZOIDFS_NONBLOCK_SERVER_IO, &nbvallen, &nbflag);
        }

        /* non-blocking IO mode */
        if(nbflag)
        {
            boost::exception_ptr e;
            unsigned int i = 0;
            int * iofsl_ret = NULL;
            void ** iofsl_mem_starts = NULL;
            size_t * iofsl_mem_sizes = NULL;
            zoidfs_file_ofs_t * iofsl_file_starts = NULL;
            zoidfs_file_size_t * iofsl_file_sizes = NULL;
            zoidfs_handle_t * iofsl_h = NULL;
            zoidfs_async_write_op_key * op_key = NULL;
            size_t total_size = 0;

            /* compute the total amount of mem required */
            for(i = 0 ; i < mem_count ; i++)
            {
                total_size += mem_sizes[i];
            }

            iofwdutil::mm::NBIOMemoryAlloc * nbio_buffer =
                new iofwdutil::mm::NBIOMemoryAlloc(total_size);
            if(iofwdutil::mm::NBIOMemoryManager::instance().try_alloc(nbio_buffer))
            {
                try
                {
                    iofsl_ret = new int;
                    *iofsl_ret = 0;

                    /* copy the handle */
                    iofsl_h = new zoidfs_handle_t;
                    memcpy(iofsl_h, handle, sizeof(zoidfs_handle_t));

                    /* copy the mem params */
                    iofsl_mem_starts = new void*[mem_count];
                    iofsl_mem_sizes = new size_t[mem_count];
                    for(i = 0 ; i < mem_count ; i++)
                    {
                        iofsl_mem_sizes[i] = mem_sizes[i];
                        iofsl_mem_starts[i] = new char[mem_sizes[i]];
                        memcpy(iofsl_mem_starts[i], mem_starts[i], mem_sizes[i]); 
                    }

                    /* copy the file params */
                    iofsl_file_starts = new zoidfs_file_ofs_t[file_count];
                    iofsl_file_sizes = new zoidfs_file_size_t[file_count];
                    for(i = 0 ; i < file_count ; i++)
                    {
                        iofsl_file_starts[i] = file_starts[i];
                        iofsl_file_sizes[i] = file_sizes[i];
                    }

                    /* track the write operation */
                    uint64_t op_key_handle_id = 0;
                    {
                        ZoidFSTrackerData * data = NULL;

                        /* find the handle in the tracker */
                        {
                            boost::mutex::scoped_lock 
                                lock(ZoidFSDefAsync::handle_tracker_mutex_);
                            ZoidFSHandleTracker::iterator it;

                            op_key_handle_id = handle_tracker_id_++;

                            /* if the handle was not in the tracker, make a new one and add
                            * it */
                            if((it = handle_tracker_.find(ZoidFSTrackerKey(iofsl_h))) == handle_tracker_.end())
                            {
                                data = new ZoidFSTrackerData();
                                handle_tracker_[ZoidFSTrackerKey(iofsl_h)] = data;
                            }
                            /* the handle data was already made, get it */
                            else
                            {
                                data = it->second;
                            }

                        }

                        /* update the handle data */
                        {
                            boost::mutex::scoped_lock lock(data->mutex_);
                            data->ref_count_++;
                        }
                    }
 
                    /* create the op key */
                    op_key = new zoidfs_async_write_op_key(*iofsl_h, op_key_handle_id);

                    /* create the zoidfs write work unit using the copied
                       params */
                    ZoidFSDefAsyncWriteWorkUnit * wu = new
                        ZoidFSDefAsyncWriteWorkUnit(cb, iofsl_ret, api_.get(), tp_,
                                iofsl_h, mem_count, const_cast<const void
                                **>(iofsl_mem_starts), iofsl_mem_sizes,
                                file_count, iofsl_file_starts,
                                iofsl_file_sizes, ZOIDFS_NO_OP_HINT, op_key,
                                nbio_buffer);

                    submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit,
                       wu), iofwdutil::ThreadPool::HIGH,
                            iofwdutil::ThreadPool::FILEIO);

                    /* set the return code to success */
                    *ret = ZFS_OK;
                }
                catch(...)
                {
                    e = boost::current_exception();
                }
                /* DO NOT invoke the callback... instead return "in progress" */
                *ret = ZFSERR_EINPROGRESS;
            }
            /* we could not allocate the buffer space for the nbio...
               switch to blocking io and ignore hint */
            else
            {
                nbflag = false;
                delete nbio_buffer;
            }
        }

        /* blocking IO mode */
        if(!nbflag)
        {
            /* create the zoidfs write work unit using the copied params */
            ZoidFSDefAsyncWriteWorkUnit * wu = new
                ZoidFSDefAsyncWriteWorkUnit(cb,
                ret, api_.get(), tp_, handle, mem_count,
                mem_starts, mem_sizes, file_count, file_starts, 
                file_sizes, hint);

            /* submit the work unit to the TP */
            if(highpriooplist_[zoidfs::ZOIDFS_PROTO_WRITE])
            {
                submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit,
                    wu), iofwdutil::ThreadPool::HIGH,
                        iofwdutil::ThreadPool::FILEIO);
            }
            else
            {
                submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit,
                    wu), iofwdutil::ThreadPool::LOW,
                        iofwdutil::ThreadPool::FILEIO);
            }
        }
   }


   void ZoidFSDefAsync::commit(const iofwdevent::CBType & cb, 
           int * ret,
           const zoidfs_handle_t * handle,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncCommitWorkUnit * wu = new ZoidFSDefAsyncCommitWorkUnit(cb, 
              ret, api_.get(), tp_, handle, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_COMMIT])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::create(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * parent_handle,
           const char * component_name,
           const char * full_path,
           const zoidfs_sattr_t * attr,
           zoidfs_handle_t * handle,
           int * created,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncCreateWorkUnit * wu = new ZoidFSDefAsyncCreateWorkUnit(cb,
              ret, api_.get(), tp_, parent_handle, component_name,
              full_path, attr, handle, created, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_CREATE])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::remove(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * parent_handle,
           const char * component_name,
           const char * full_path,
           zoidfs_cache_hint_t * parent_hint,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncRemoveWorkUnit * wu = new ZoidFSDefAsyncRemoveWorkUnit(cb,
              ret, api_.get(), tp_, parent_handle, component_name,
              full_path, parent_hint, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_REMOVE])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }

   void ZoidFSDefAsync::rename(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * from_parent_handle,
           const char * from_component_name,
           const char * from_full_path,
           const zoidfs_handle_t * to_parent_handle,
           const char * to_component_name,
           const char * to_full_path,
           zoidfs_cache_hint_t * from_parent_hint,
           zoidfs_cache_hint_t * to_parent_hint,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncRenameWorkUnit * wu = new ZoidFSDefAsyncRenameWorkUnit(cb,
              ret, api_.get(), tp_, from_parent_handle,
              from_component_name, from_full_path, to_parent_handle,
              to_component_name, to_full_path, from_parent_hint,
              to_parent_hint, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_RENAME])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }

   void ZoidFSDefAsync::link(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * from_parent_handle,
           const char * from_component_name,
           const char * from_full_path,
           const zoidfs_handle_t * to_parent_handle,
           const char * to_component_name,
           const char * to_full_path,
           zoidfs_cache_hint_t * from_parent_hint,
           zoidfs_cache_hint_t * to_parent_hint,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncLinkWorkUnit * wu = new ZoidFSDefAsyncLinkWorkUnit(cb,
              ret, api_.get(), tp_, from_parent_handle, from_component_name, 
              from_full_path, to_parent_handle, to_component_name,
              to_full_path, from_parent_hint, to_parent_hint, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_LINK])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }

   void ZoidFSDefAsync::symlink(const iofwdevent::CBType & cb,
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
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncSymlinkWorkUnit * wu = new ZoidFSDefAsyncSymlinkWorkUnit(cb,
              ret, api_.get(), tp_, from_parent_handle, from_component_name, 
            from_full_path, to_parent_handle, to_component_name, to_full_path,
            attr, from_parent_hint, to_parent_hint, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_SYMLINK])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::mkdir(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * parent_handle,
           const char * component_name,
           const char * full_path,
           const zoidfs_sattr_t * attr,
           zoidfs_cache_hint_t * parent_hint,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncMkdirWorkUnit * wu = new ZoidFSDefAsyncMkdirWorkUnit(cb,
              ret, api_.get(), tp_, parent_handle, component_name,
              full_path, attr, parent_hint, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_MKDIR])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::readdir(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * parent_handle,
           zoidfs_dirent_cookie_t cookie,
           size_t * entry_count,
           zoidfs_dirent_t * entries,
           uint32_t flags,
           zoidfs_cache_hint_t * parent_hint,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncReaddirWorkUnit * wu = new ZoidFSDefAsyncReaddirWorkUnit(cb,
              ret, api_.get(), tp_, parent_handle, cookie, entry_count,
              entries, flags, parent_hint, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_READDIR])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }


   void ZoidFSDefAsync::resize(const iofwdevent::CBType & cb,
           int * ret,
           const zoidfs_handle_t * handle,
           zoidfs_file_size_t size,
           zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncResizeWorkUnit * wu = new ZoidFSDefAsyncResizeWorkUnit(cb,
              ret, api_.get(), tp_, handle, size, hint);

      if(highpriooplist_[zoidfs::ZOIDFS_PROTO_RESIZE])
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::HIGH, iofwdutil::ThreadPool::FILEIO);
      else
        submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu),
                iofwdutil::ThreadPool::LOW, iofwdutil::ThreadPool::FILEIO);
   }

    } /* namespace util */
} /* namespace zoidfs */
