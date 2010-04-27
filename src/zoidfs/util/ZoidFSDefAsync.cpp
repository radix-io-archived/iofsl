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
      }
      catch (iofwdutil::FactoryException & e)
      {
         ZLOG_ERROR(log_, format("Could not instantiate blocking API '%s'!") %
               apiname);
         // TODO: translate exception?
         throw;
      }

      wait_for_threads_ = config.getKeyAsDefault<bool>("use_thread_pool", false);
      if (wait_for_threads_)
      {
         ZLOG_INFO(log_, "Using thread pool...");
      }
      else
      {
         ZLOG_INFO(log_, "Not using thread pool...");
      }

      // Configure blocking api_
      Configurable::configure_if_needed (api_.get(),
            config.openSectionDefault(apiname.c_str()));
   }


   void ZoidFSDefAsync::null(const iofwdevent::CBType & cb, int * ret)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::null, boost::ref(*api_)));
   }


   void ZoidFSDefAsync::getattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * op_hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::getattr, boost::ref(*api_), handle, attr,
               op_hint));
   }


   void ZoidFSDefAsync::setattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         const zoidfs_sattr_t * sattr,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::setattr, boost::ref(*api_), handle, sattr, attr,
               hint));
   }


   void ZoidFSDefAsync::lookup(const iofwdevent::CBType & cb, int * ret,
         const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         zoidfs_handle_t * handle,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::lookup, boost::ref(*api_), parent_handle, component_name,
               full_path, handle, hint));
   }


   void ZoidFSDefAsync::readlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         char * buffer,
         size_t buffer_length,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::readlink, boost::ref(*api_), handle, buffer, buffer_length,
               hint));
   }


   void ZoidFSDefAsync::read(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         size_t mem_count,
         void * mem_starts[],
         const size_t mem_sizes[],
         size_t file_count,
         const uint64_t file_starts[],
         uint64_t file_sizes[],
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::read, boost::ref(*api_), handle, mem_count, mem_starts,
               mem_sizes, file_count, file_starts, file_sizes, hint));
   }


   void ZoidFSDefAsync::write(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         size_t mem_count,
         const void * mem_starts[],
         const size_t mem_sizes[],
         size_t file_count,
         const uint64_t file_starts[],
         uint64_t file_sizes[],
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::write, boost::ref(*api_), handle, mem_count, mem_starts,
               mem_sizes, file_count, file_starts, file_sizes, hint));
   }


   void ZoidFSDefAsync::commit(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::commit, boost::ref(*api_), handle, hint));
   }


   void ZoidFSDefAsync::create(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         const zoidfs_sattr_t * attr,
         zoidfs_handle_t * handle,
         int * created,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::create, boost::ref(*api_), parent_handle, component_name,
               full_path, attr, handle, created, hint));
   }


   void ZoidFSDefAsync::remove(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::remove, boost::ref(*api_), parent_handle, component_name,
               full_path, parent_hint, hint));
   }

   int ZoidFSDefAsync::rename_helper(rename_helper_bundle_t * b)
   {
        /* run the op */
        int ret = api_->rename(b->from_parent_handle_,
            b->from_component_name_, b->from_full_path_, b->to_parent_handle_,
            b->to_component_name_, b->to_full_path_, b->from_parent_hint_, b->to_parent_hint_,
            b->hint_);

        /* destroy the bundle */
        delete b;

        /* return the op result */
        return ret;
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
      addWork (cb, ret, boost::bind(&ZoidFSDefAsync::rename_helper, this, new rename_helper_bundle_t(from_parent_handle,
               from_component_name, from_full_path, to_parent_handle,
               to_component_name, to_full_path, from_parent_hint, to_parent_hint,
               hint)));
   }

   int ZoidFSDefAsync::link_helper(link_helper_bundle_t * b2)
   {
        // use auto_ptr for exception safety
      boost::scoped_ptr<link_helper_bundle_t> b(b2);

        /* run the op */
        int ret = api_->link(b->from_parent_handle_,
            b->from_component_name_, b->from_full_path_, b->to_parent_handle_,
            b->to_component_name_, b->to_full_path_, b->from_parent_hint_, b->to_parent_hint_,
            b->hint_);

        /* return the op result */
        return ret;
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
      addWork (cb, ret, boost::bind(&ZoidFSDefAsync::link_helper, this,
               new link_helper_bundle_t(from_parent_handle, from_component_name, from_full_path,
               to_parent_handle, to_component_name, to_full_path, from_parent_hint,
               to_parent_hint, hint)));
   }

   int ZoidFSDefAsync::symlink_helper(symlink_helper_bundle_t * b2)
   {
      boost::scoped_ptr<symlink_helper_bundle_t> b(b2);

        /* run the op */
        int ret = api_->symlink(b->from_parent_handle_,
            b->from_component_name_, b->from_full_path_, b->to_parent_handle_,
            b->to_component_name_, b->to_full_path_, b->attr_, b->from_parent_hint_, b->to_parent_hint_,
            b->hint_);

        /* return the op result */
        return ret;
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
      addWork (cb, ret, boost::bind(&ZoidFSDefAsync::symlink_helper, this, new symlink_helper_bundle_t(from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name, to_full_path,
               attr, from_parent_hint, to_parent_hint, hint)));
   }


   void ZoidFSDefAsync::mkdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         const zoidfs_sattr_t * attr,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::mkdir, boost::ref(*api_), parent_handle, component_name,
               full_path, attr, parent_hint, hint));
   }


   void ZoidFSDefAsync::readdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         zoidfs_dirent_cookie_t cookie,
         size_t * entry_count,
         zoidfs_dirent_t * entries,
         uint32_t flags,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::readdir, boost::ref(*api_), parent_handle, cookie,
               entry_count, entries, flags, parent_hint, hint));
   }


   void ZoidFSDefAsync::resize(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         uint64_t size,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::resize, boost::ref(*api_), handle, size, hint));
   }

    } /* namespace util */
} /* namespace zoidfs */
