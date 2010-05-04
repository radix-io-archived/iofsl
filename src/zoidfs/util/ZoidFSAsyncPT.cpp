#include <boost/format.hpp>

#include "ZoidFSAsyncPT.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/Factory.hh"
#include "iofwdutil/Configurable.hh"

using boost::format;

namespace zoidfs
{
   namespace util
   {
      //=====================================================================

      ZoidFSAsyncPT::ZoidFSAsyncPT (ZoidFSAsync * pt)
         : pt_(pt)
      {
      }


      void ZoidFSAsyncPT::setAsyncPT (ZoidFSAsync * pt)
      {
         ALWAYS_ASSERT(pt);
         pt_ = pt;
      }

      void ZoidFSAsyncPT::configurePT (iofwdutil::IOFWDLogSource & log,
            const iofwdutil::ConfigFile & config)
      {
         const std::string api (config.getKeyDefault ("api", ""));
         if (!api.size())
         {
            ZLOG_ERROR(log, format("No 'api' specified in config file!"));
            throw "No API specified!";
         }
         ZLOG_INFO (log, format("Using async api '%s'") % api);
         api_.reset (iofwdutil::Factory<
               std::string,
               zoidfs::util::ZoidFSAsync>::construct(api)());
         iofwdutil::Configurable::configure_if_needed (api_.get(),
               config.openSectionDefault(api.c_str()));

         setAsyncPT (api_.get());
      }

      int ZoidFSAsyncPT::init(void)
      {
         return pt_->init ();
      }


      int ZoidFSAsyncPT::finalize(void)
      {
         return pt_->finalize ();
      }


      void ZoidFSAsyncPT::null(const iofwdevent::CBType & cb, int * ret)
      {
         pt_->null (cb, ret);
      }


      void ZoidFSAsyncPT::getattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * op_hint)
      {
         pt_->getattr(cb, ret, handle, attr, op_hint);
      }


      void ZoidFSAsyncPT::setattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * sattr,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * hint)
      {
         pt_->setattr(cb, ret, handle, sattr, attr, hint);
      }


      void ZoidFSAsyncPT::lookup(const iofwdevent::CBType & cb, int * ret,
            const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint)
      {
         pt_->lookup (cb, ret, parent_handle, component_name, full_path,
               handle, hint);
      }


      void ZoidFSAsyncPT::readlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length,
            zoidfs_op_hint_t * hint)
      {
         pt_->readlink (cb, ret, handle, buffer, buffer_length, hint);
      }


      void ZoidFSAsyncPT::read(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         pt_->read (cb, ret, handle, mem_count, mem_starts, mem_sizes, file_count,
               file_starts, file_sizes, hint);
      }


      void ZoidFSAsyncPT::write(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         pt_->write (cb, ret, handle, mem_count, mem_starts, mem_sizes,
               file_count, file_starts, file_sizes, hint);
      }


      void ZoidFSAsyncPT::commit(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint)
      {
         pt_->commit (cb, ret, handle, hint);
      }


      void ZoidFSAsyncPT::create(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created,
            zoidfs_op_hint_t * hint)
      {
         pt_->create (cb, ret, parent_handle, component_name, full_path, attr,
               handle, created, hint);
      }


      void ZoidFSAsyncPT::remove(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
      {
         pt_->remove (cb, ret, parent_handle, component_name, full_path,
               parent_hint, hint);
      }


      void ZoidFSAsyncPT::rename(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint)
      {
         pt_->rename (cb, ret, from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name,
               to_full_path, from_parent_hint, to_parent_hint, hint);
      }


      void ZoidFSAsyncPT::link(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint)
      {
         pt_->link (cb, ret, from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name,
               to_full_path, from_parent_hint, to_parent_hint, hint);
      }



      void ZoidFSAsyncPT::symlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
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
         pt_->symlink (cb, ret, from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name,
               to_full_path, attr, from_parent_hint, to_parent_hint, hint);
      }


      void ZoidFSAsyncPT::mkdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
      {
         pt_->mkdir (cb, ret, parent_handle, component_name, full_path, attr,
               parent_hint, hint);
      }


      void ZoidFSAsyncPT::readdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
      {
         pt_->readdir (cb, ret, parent_handle, cookie, entry_count, entries,
               flags, parent_hint, hint);
      }


      void ZoidFSAsyncPT::resize(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            zoidfs_file_size_t size,
            zoidfs_op_hint_t * hint)
      {
         pt_->resize (cb, ret, handle, size, hint);
      }

      //=====================================================================
   }
}
