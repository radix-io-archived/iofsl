#ifndef ZOIDFS_UTIL_ZOIDFSBLOCKASYNC_HH
#define ZOIDFS_UTIL_ZOIDFSBLOCKASYNC_HH

#include "ZoidFSAPI.hh"
#include "ZoidFSAsync.hh"

#include <boost/noncopyable.hpp>

namespace zoidfs
{
   namespace util
   {
      /**
       * Async implementation that calls the blocking zoidfs functions in the
       * context of the caller. As such, it is not async at all.
       */
      class ZoidFSBlockAsync  : public ZoidFSAsync, private boost::noncopyable
      {
         ZoidFSBlockAsync (ZoidFSAPI * delegate);

         virtual int init(void) ;

         virtual int finalize(void);

         virtual void null(const iofwdevent::CBType & cb, int * ret) ;

         virtual void getattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
               zoidfs_attr_t * attr,
               zoidfs_op_hint_t * op_hint) ;

         virtual void setattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
               const zoidfs_sattr_t * sattr,
               zoidfs_attr_t * attr,
               zoidfs_op_hint_t * hint) ;

         virtual void lookup(const iofwdevent::CBType & cb, int * ret,
               const zoidfs_handle_t * parent_handle,
               const char * component_name,
               const char * full_path,
               zoidfs_handle_t * handle,
               zoidfs_op_hint_t * hint) ;

         virtual void readlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
               char * buffer,
               size_t buffer_length,
               zoidfs_op_hint_t * hint) ;

         virtual void read(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
               size_t mem_count,
               void * mem_starts[],
               const size_t mem_sizes[],
               size_t file_count,
               const zoidfs_file_ofs_t file_starts[],
               zoidfs_file_size_t file_sizes[],
               zoidfs_op_hint_t * hint) ;

         virtual void write(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
               size_t mem_count,
               const void * mem_starts[],
               const size_t mem_sizes[],
               size_t file_count,
               const zoidfs_file_ofs_t file_starts[],
               zoidfs_file_size_t file_sizes[],
               zoidfs_op_hint_t * hint) ;

         virtual void commit(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
               zoidfs_op_hint_t * hint) ;

         virtual void create(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
               const char * component_name,
               const char * full_path,
               const zoidfs_sattr_t * attr,
               zoidfs_handle_t * handle,
               int * created,
               zoidfs_op_hint_t * hint) ;

         virtual void remove(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
               const char * component_name,
               const char * full_path,
               zoidfs_cache_hint_t * parent_hint,
               zoidfs_op_hint_t * hint) ;

         virtual void rename(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
               const char * from_component_name,
               const char * from_full_path,
               const zoidfs_handle_t * to_parent_handle,
               const char * to_component_name,
               const char * to_full_path,
               zoidfs_cache_hint_t * from_parent_hint,
               zoidfs_cache_hint_t * to_parent_hint,
               zoidfs_op_hint_t * hint) ;

         virtual void link(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
               const char * from_component_name,
               const char * from_full_path,
               const zoidfs_handle_t * to_parent_handle,
               const char * to_component_name,
               const char * to_full_path,
               zoidfs_cache_hint_t * from_parent_hint,
               zoidfs_cache_hint_t * to_parent_hint,
               zoidfs_op_hint_t * hint) ;


         virtual void symlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
               const char * from_component_name,
               const char * from_full_path,
               const zoidfs_handle_t * to_parent_handle,
               const char * to_component_name,
               const char * to_full_path,
               const zoidfs_sattr_t * attr,
               zoidfs_cache_hint_t * from_parent_hint,
               zoidfs_cache_hint_t * to_parent_hint,
               zoidfs_op_hint_t * hint) ;

         virtual void mkdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
               const char * component_name,
               const char * full_path,
               const zoidfs_sattr_t * attr,
               zoidfs_cache_hint_t * parent_hint,
               zoidfs_op_hint_t * hint) ;

         virtual void readdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
               zoidfs_dirent_cookie_t cookie,
               size_t * entry_count,
               zoidfs_dirent_t * entries,
               uint32_t flags,
               zoidfs_cache_hint_t * parent_hint,
               zoidfs_op_hint_t * hint) ;

         virtual void resize(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
               zoidfs_file_size_t size,
               zoidfs_op_hint_t * hint) ;

       protected:

         ZoidFSAPI * delegate_;
      };
   }
}

#endif
