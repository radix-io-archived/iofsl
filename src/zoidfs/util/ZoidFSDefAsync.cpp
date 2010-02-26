#include "ZoidFSDefAsync.hh"
#include <boost/bind.hpp>
#include "iofwdutil/tools.hh"

namespace zoidfs
{
    namespace util
    {

   //=====================================================================

   int ZoidFSDefAsync::init(void)
   {
      return 1;
   }

   int ZoidFSDefAsync::finalize(void)
   {
      return 1;
   }

   void ZoidFSDefAsync::null(const iofwdevent::CBType & cb, int * ret)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::null, boost::ref(api_)));
   }


   void ZoidFSDefAsync::getattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * op_hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::getattr, boost::ref(api_), handle, attr,
               op_hint));
   }


   void ZoidFSDefAsync::setattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         const zoidfs_sattr_t * sattr,
         zoidfs_attr_t * attr,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::setattr, boost::ref(api_), handle, sattr, attr,
               hint));
   }


   void ZoidFSDefAsync::lookup(const iofwdevent::CBType & cb, int * ret,
         const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         zoidfs_handle_t * handle,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::lookup, boost::ref(api_), parent_handle, component_name,
               full_path, handle, hint));
   }


   void ZoidFSDefAsync::readlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         char * buffer,
         size_t buffer_length,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::readlink, boost::ref(api_), handle, buffer, buffer_length,
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
      addWork (cb, ret, boost::bind(&ZoidFSAPI::read, boost::ref(api_), handle, mem_count, mem_starts,
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
      addWork (cb, ret, boost::bind(&ZoidFSAPI::write, boost::ref(api_), handle, mem_count, mem_starts,
               mem_sizes, file_count, file_starts, file_sizes, hint));
   }


   void ZoidFSDefAsync::commit(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::commit, boost::ref(api_), handle, hint));
   }


   void ZoidFSDefAsync::create(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         const zoidfs_sattr_t * attr,
         zoidfs_handle_t * handle,
         int * created,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::create, boost::ref(api_), parent_handle, component_name,
               full_path, attr, handle, created, hint));
   }


   void ZoidFSDefAsync::remove(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::remove, boost::ref(api_), parent_handle, component_name,
               full_path, parent_hint, hint));
   }

   /* TODO: boost::bind is limited to 9 function arguments... the the following bind does not work... */
   void ZoidFSDefAsync::rename(const iofwdevent::CBType & UNUSED(cb), int * UNUSED(ret), const zoidfs_handle_t * UNUSED(from_parent_handle),
         const char * UNUSED(from_component_name),
         const char * UNUSED(from_full_path),
         const zoidfs_handle_t * UNUSED(to_parent_handle),
         const char * UNUSED(to_component_name),
         const char * UNUSED(to_full_path),
         zoidfs_cache_hint_t * UNUSED(from_parent_hint),
         zoidfs_cache_hint_t * UNUSED(to_parent_hint),
         zoidfs_op_hint_t * UNUSED(hint))
   {
      /*addWork (cb, ret, boost::bind(&ZoidFSAPI::rename, boost::ref(api_), from_parent_handle,
               from_component_name, from_full_path, to_parent_handle,
               to_component_name, to_full_path, from_parent_hint, to_parent_hint,
               hint));*/
   }

   /* TODO: boost::bind is limited to 9 function arguments... the the following bind does not work... */
   void ZoidFSDefAsync::link(const iofwdevent::CBType & UNUSED(cb), int * UNUSED(ret), const zoidfs_handle_t * UNUSED(from_parent_handle),
         const char * UNUSED(from_component_name),
         const char * UNUSED(from_full_path),
         const zoidfs_handle_t * UNUSED(to_parent_handle),
         const char * UNUSED(to_component_name),
         const char * UNUSED(to_full_path),
         zoidfs_cache_hint_t * UNUSED(from_parent_hint),
         zoidfs_cache_hint_t * UNUSED(to_parent_hint),
         zoidfs_op_hint_t * UNUSED(hint))
   {
      /*addWork (cb, ret, boost::bind(&ZoidFSAPI::link, boost::ref(api_),
               from_parent_handle, from_component_name, from_full_path,
               to_parent_handle, to_component_name, to_full_path, from_parent_hint,
               to_parent_hint, hint));*/
   }

   /* TODO: boost::bind is limited to 9 function arguments... the the following bind does not work... */
   void ZoidFSDefAsync::symlink(const iofwdevent::CBType & UNUSED(cb), int * UNUSED(ret), const zoidfs_handle_t * UNUSED(from_parent_handle),
         const char * UNUSED(from_component_name),
         const char * UNUSED(from_full_path),
         const zoidfs_handle_t * UNUSED(to_parent_handle),
         const char * UNUSED(to_component_name),
         const char * UNUSED(to_full_path),
         const zoidfs_sattr_t * UNUSED(attr),
         zoidfs_cache_hint_t * UNUSED(from_parent_hint),
         zoidfs_cache_hint_t * UNUSED(to_parent_hint),
         zoidfs_op_hint_t * UNUSED(hint))
   {
      /*addWork (cb, ret, boost::bind(&ZoidFSAPI::setattr, boost::ref(api_), from_parent_handle, from_component_name,
               from_full_path, to_parent_handle, to_component_name, to_full_path,
               attr, from_parent_hint, to_parent_hint, hint));*/
   }


   void ZoidFSDefAsync::mkdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
         const char * component_name,
         const char * full_path,
         const zoidfs_sattr_t * attr,
         zoidfs_cache_hint_t * parent_hint,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::mkdir, boost::ref(api_), parent_handle, component_name,
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
      addWork (cb, ret, boost::bind(&ZoidFSAPI::readdir, boost::ref(api_), parent_handle, cookie,
               entry_count, entries, flags, parent_hint, hint));
   }


   void ZoidFSDefAsync::resize(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
         uint64_t size,
         zoidfs_op_hint_t * hint)
   {
      addWork (cb, ret, boost::bind(&ZoidFSAPI::resize, boost::ref(api_), handle, size, hint));
   }

    } /* namespace util */
} /* namespace zoidfs */
