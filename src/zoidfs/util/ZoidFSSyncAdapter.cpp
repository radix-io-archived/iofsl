#include <boost/format.hpp>

#include "ZoidFSSyncAdapter.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdutil/Factory.hh"
#include "iofwdutil/LinkHelper.hh"

using iofwdevent::SingleCompletion;
using boost::format;

GENERIC_FACTORY_CLIENT(std::string,
      zoidfs::util::ZoidFSAPI,
      zoidfs::util::ZoidFSSyncAdapter,
      "syncadapter",
      syncadapter);

namespace zoidfs
{
   namespace util
   {
      //=====================================================================

      ZoidFSSyncAdapter::ZoidFSSyncAdapter ()
         : log_ (iofwdutil::IOFWDLog::getSource ("zoidfssyncadapter"))
      {
      }

      void ZoidFSSyncAdapter::configure (const iofwdutil::ConfigFile & file)
      {
         std::string api = file.getKeyDefault ("api", "defasync");
         ZLOG_INFO (log_, format("Using async API '%s'") % api);
         api_.reset (iofwdutil::Factory<
                        std::string,
                        zoidfs::util::ZoidFSAsync>::construct (api)());
         iofwdutil::Configurable::configure_if_needed (api_.get(),
               file.openSectionDefault(api.c_str()));
      }

      int ZoidFSSyncAdapter::init()
      {
         return api_->init ();
      }


      int ZoidFSSyncAdapter::finalize()
      {
         return api_->finalize ();
      }


      int ZoidFSSyncAdapter::null()
      {
         SingleCompletion block;
         int ret;
         api_->null (block, &ret);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::getattr(const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * op_hint)
      {
         SingleCompletion block;
         int ret;
         api_->getattr (block, &ret, handle, attr, op_hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::setattr(const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * sattr,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->setattr (block, &ret, handle, sattr, attr, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::lookup(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->lookup (block, &ret, parent_handle, component_name, full_path,
               handle, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::readlink(const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->readlink (block, &ret, handle, buffer, buffer_length, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::read(const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->read (block, &ret, handle, mem_count, mem_starts, mem_sizes,
               file_count, file_starts, file_sizes, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::write(const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const uint64_t file_starts[],
            uint64_t file_sizes[],
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->write (block, &ret, handle, mem_count, mem_starts, mem_sizes,
               file_count, file_starts, file_sizes, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::commit(const zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->commit (block, &ret, handle, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::create(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->create (block, &ret, parent_handle, component_name, full_path,
               attr, handle, created, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::remove(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->remove (block, &ret, parent_handle, component_name, full_path,
               parent_hint, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::rename(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->rename (block, &ret, from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name,
               to_full_path, from_parent_hint, to_parent_hint, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::link(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->link (block, &ret, from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name,
               to_full_path, from_parent_hint, to_parent_hint, hint);
         block.wait ();
         return ret;
      }



      int ZoidFSSyncAdapter::symlink(const zoidfs_handle_t * from_parent_handle,
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
         SingleCompletion block;
         int ret;
         api_->symlink (block, &ret, from_parent_handle, from_component_name, 
               from_full_path, to_parent_handle, to_component_name,
               to_full_path,
               attr, from_parent_hint, to_parent_hint, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::mkdir(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->mkdir (block, &ret, parent_handle, component_name, full_path,
               attr, parent_hint, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::readdir(const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->readdir (block, &ret, parent_handle, cookie, entry_count,
               entries, flags, parent_hint, hint);
         block.wait ();
         return ret;
      }


      int ZoidFSSyncAdapter::resize(const zoidfs_handle_t * handle,
            uint64_t size,
            zoidfs_op_hint_t * hint)
      {
         SingleCompletion block;
         int ret;
         api_->resize (block, &ret, handle, size, hint);
         block.wait ();
         return ret;
      }


      //=====================================================================
   }
}
