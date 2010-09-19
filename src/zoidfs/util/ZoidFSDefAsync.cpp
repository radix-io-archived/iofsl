#include <memory>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/format.hpp>

#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/tools.hh"
#include "ZoidFSDefAsync.hh"
#include "iofwdutil/ConfigFile.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

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


   void ZoidFSDefAsync::runWorkUnit(ZoidFSDefAsyncWorkUnit * bwu)
   {
       switch(bwu->type_)
       {
           case zoidfs::ZOIDFS_PROTO_GET_ATTR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncGetattrWorkUnit * wu = static_cast<ZoidFSDefAsyncGetattrWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->getattr(wu->handle_, wu->attr_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_SET_ATTR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncSetattrWorkUnit * wu = static_cast<ZoidFSDefAsyncSetattrWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->setattr(wu->handle_, wu->sattr_, wu->attr_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_LOOKUP:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncLookupWorkUnit * wu = static_cast<ZoidFSDefAsyncLookupWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->lookup(wu->parent_handle_, wu->component_name_, wu->full_path_, wu->handle_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_READLINK:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncReadlinkWorkUnit * wu = static_cast<ZoidFSDefAsyncReadlinkWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->readlink(wu->handle_, wu->buffer_, wu->buffer_length_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_COMMIT:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncCommitWorkUnit * wu = static_cast<ZoidFSDefAsyncCommitWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->commit(wu->handle_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_CREATE:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncCreateWorkUnit * wu = static_cast<ZoidFSDefAsyncCreateWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->create(wu->parent_handle_, wu->component_name_, wu->full_path_, wu->sattr_, wu->handle_, wu->created_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_REMOVE:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncRemoveWorkUnit * wu = static_cast<ZoidFSDefAsyncRemoveWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->remove(wu->parent_handle_, wu->component_name_, wu->full_path_, wu->parent_hint_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_RENAME:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncRenameWorkUnit * wu = static_cast<ZoidFSDefAsyncRenameWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->rename(wu->from_parent_handle_, wu->from_component_name_, wu->from_full_path_, wu->to_parent_handle_, wu->to_component_name_, wu->to_full_path_,
                    wu->from_parent_hint_, wu->to_parent_hint_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_LINK:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncLinkWorkUnit * wu = static_cast<ZoidFSDefAsyncLinkWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->link(wu->from_parent_handle_, wu->from_component_name_, wu->from_full_path_, wu->to_parent_handle_, wu->to_component_name_, wu->to_full_path_,
                    wu->from_parent_hint_, wu->to_parent_hint_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_SYMLINK:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncSymlinkWorkUnit * wu = static_cast<ZoidFSDefAsyncSymlinkWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->symlink(wu->from_parent_handle_, wu->from_component_name_, wu->from_full_path_, wu->to_parent_handle_, wu->to_component_name_, wu->to_full_path_,
                    wu->sattr_, wu->from_parent_hint_, wu->to_parent_hint_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_MKDIR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncMkdirWorkUnit * wu = static_cast<ZoidFSDefAsyncMkdirWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->mkdir(wu->parent_handle_, wu->component_name_, wu->full_path_, wu->sattr_, wu->parent_hint_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_READDIR:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncReaddirWorkUnit * wu = static_cast<ZoidFSDefAsyncReaddirWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->readdir(wu->parent_handle_, wu->cookie_, wu->entry_count_, wu->entries_, wu->flags_, wu->parent_hint_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_RESIZE:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncResizeWorkUnit * wu = static_cast<ZoidFSDefAsyncResizeWorkUnit *>(bwu);

              *(wu->ret_) = wu->api_->resize(wu->handle_, wu->size_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           case zoidfs::ZOIDFS_PROTO_READ:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncReadWorkUnit * wu = static_cast<ZoidFSDefAsyncReadWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->read(wu->handle_, wu->mem_count_,
                                wu->mem_starts_, wu->mem_sizes_,
                                wu->file_count_, wu->file_starts_,
                                wu->file_sizes_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }

           case zoidfs::ZOIDFS_PROTO_WRITE:
           {
              /* convert to the write work unit */
              ZoidFSDefAsyncWriteWorkUnit * wu = static_cast<ZoidFSDefAsyncWriteWorkUnit *>(bwu);

              /* invoke the write API */
              *(wu->ret_) = wu->api_->write(wu->handle_, wu->mem_count_,
                                wu->mem_starts_, wu->mem_sizes_,
                                wu->file_count_, wu->file_starts_,
                                wu->file_sizes_, wu->hint_);

              /* invoke the callback */
              wu->cb_(iofwdevent::COMPLETED);

              break;
           }
           default:
           {
              break;
           }
       };

       /* reschedule the thread with more work from the tp */
       boost::this_thread::at_thread_exit(iofwdutil::ThreadPoolKick(bwu->tp_));
       //iofwdutil::ThreadPoolKick(bwu->tp_);

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
   }


   void ZoidFSDefAsync::null(const iofwdevent::CBType & cb, int * ret)
   {
      ZoidFSDefAsyncNullWorkUnit * wu = new ZoidFSDefAsyncNullWorkUnit(cb, ret, api_.get(), tp_);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::getattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * op_hint)
   {
      ZoidFSDefAsyncGetattrWorkUnit * wu = new ZoidFSDefAsyncGetattrWorkUnit(cb, ret, api_.get(), tp_, handle, attr, op_hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::setattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         const zoidfs_sattr_t * sattr,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncSetattrWorkUnit * wu = new ZoidFSDefAsyncSetattrWorkUnit(cb, ret, api_.get(), tp_, handle, sattr, attr, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::lookup(const iofwdevent::CBType & cb, int * ret,
         const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         zoidfs_handle_t * handle,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncLookupWorkUnit * wu = new ZoidFSDefAsyncLookupWorkUnit(cb, ret, api_.get(), tp_, parent_handle, component_name, full_path, handle, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::readlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         char * buffer,
         size_t buffer_length,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncReadlinkWorkUnit * wu = new ZoidFSDefAsyncReadlinkWorkUnit(cb, ret, api_.get(), tp_, handle, buffer, buffer_length, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::read(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         size_t mem_count,
         void * mem_starts[],
         const size_t mem_sizes[],
         size_t file_count,
         const zoidfs_file_ofs_t file_starts[],
         const zoidfs_file_size_t file_sizes[],
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncReadWorkUnit * wu = new ZoidFSDefAsyncReadWorkUnit(cb, ret, api_.get(), tp_, handle, mem_count, mem_starts, mem_sizes, file_count, file_starts, file_sizes, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::write(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         size_t mem_count,
         const void * mem_starts[],
         const size_t mem_sizes[],
         size_t file_count,
         const zoidfs_file_ofs_t file_starts[],
         const zoidfs_file_size_t file_sizes[],
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncWriteWorkUnit * wu = new ZoidFSDefAsyncWriteWorkUnit(cb, ret, api_.get(), tp_, handle, mem_count, mem_starts, mem_sizes, file_count, file_starts, file_sizes, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::HIGH);
   }


   void ZoidFSDefAsync::commit(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncCommitWorkUnit * wu = new ZoidFSDefAsyncCommitWorkUnit(cb, ret, api_.get(), tp_, handle, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::create(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         const zoidfs_sattr_t * attr,
         zoidfs_handle_t * handle,
         int * created,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncCreateWorkUnit * wu = new ZoidFSDefAsyncCreateWorkUnit(cb, ret, api_.get(), tp_, parent_handle, component_name, full_path, attr, handle, created, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::remove(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncRemoveWorkUnit * wu = new ZoidFSDefAsyncRemoveWorkUnit(cb, ret, api_.get(), tp_, parent_handle, component_name, full_path, parent_hint, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }

   void ZoidFSDefAsync::rename(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
         const char * from_component_name,
         const char * from_full_path,
         const zoidfs_handle_t * to_parent_handle,
         const char * to_component_name,
         const char * to_full_path,
         zoidfs_cache_hint_t * from_parent_hint,
         zoidfs_cache_hint_t * to_parent_hint,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncRenameWorkUnit * wu = new ZoidFSDefAsyncRenameWorkUnit(cb, ret, api_.get(), tp_, from_parent_handle, from_component_name, 
            from_full_path, to_parent_handle, to_component_name, to_full_path, from_parent_hint, to_parent_hint, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }

   void ZoidFSDefAsync::link(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
         const char * from_component_name,
         const char * from_full_path,
         const zoidfs_handle_t * to_parent_handle,
         const char * to_component_name,
         const char * to_full_path,
         zoidfs_cache_hint_t * from_parent_hint,
         zoidfs_cache_hint_t * to_parent_hint,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncLinkWorkUnit * wu = new ZoidFSDefAsyncLinkWorkUnit(cb, ret, api_.get(), tp_, from_parent_handle, from_component_name, 
            from_full_path, to_parent_handle, to_component_name, to_full_path, from_parent_hint, to_parent_hint, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }

   void ZoidFSDefAsync::symlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
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
      ZoidFSDefAsyncSymlinkWorkUnit * wu = new ZoidFSDefAsyncSymlinkWorkUnit(cb, ret, api_.get(), tp_, from_parent_handle, from_component_name, 
            from_full_path, to_parent_handle, to_component_name, to_full_path, attr, from_parent_hint, to_parent_hint, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::mkdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         const zoidfs_sattr_t * attr,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncMkdirWorkUnit * wu = new ZoidFSDefAsyncMkdirWorkUnit(cb, ret, api_.get(), tp_, parent_handle, component_name, full_path, attr, parent_hint, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::readdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         zoidfs_dirent_cookie_t cookie,
         size_t * entry_count,
         zoidfs_dirent_t * entries,
         uint32_t flags,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncReaddirWorkUnit * wu = new ZoidFSDefAsyncReaddirWorkUnit(cb, ret, api_.get(), tp_, parent_handle, cookie, entry_count, entries, flags, parent_hint, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }


   void ZoidFSDefAsync::resize(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         zoidfs_file_size_t size,
         zoidfs_op_hint_t * hint)
   {
      ZoidFSDefAsyncResizeWorkUnit * wu = new ZoidFSDefAsyncResizeWorkUnit(cb, ret, api_.get(), tp_, handle, size, hint);

      submitWorkUnit(boost::bind(&ZoidFSDefAsync::runWorkUnit, wu), iofwdutil::ThreadPool::LOW);
   }

    } /* namespace util */
} /* namespace zoidfs */
