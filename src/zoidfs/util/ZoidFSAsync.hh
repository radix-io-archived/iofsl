#ifndef __ZOIDFS_UTIL_ZOIDFSASYNC_HH__
#define __ZOIDFS_UTIL_ZOIDFSASYNC_HH__

#include "zoidfs-wrapped.hh"
#include "iofwdevent/CBType.hh"

namespace zoidfs
{
    namespace util
    {
//===========================================================================

/**
 * Class encapsulated interface to the ZoidFS API.
 */
class ZoidFSAsync
{
   public:
      virtual int init(void) =0;

      virtual int finalize(void) = 0;

      virtual void null(const iofwdevent::CBType & cb, int * ret) =0;

      virtual void getattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * op_hint) =0;

      virtual void setattr(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * sattr,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * hint) =0;

      virtual void lookup(const iofwdevent::CBType & cb, int * ret,
            const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint) =0;

      virtual void readlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length,
            zoidfs_op_hint_t * hint) =0;

      virtual void read(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const uint64_t file_starts[],
            uint64_t file_sizes[],
            zoidfs_op_hint_t * hint) =0;

      virtual void write(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const uint64_t file_starts[],
            uint64_t file_sizes[],
            zoidfs_op_hint_t * hint) =0;

      virtual void commit(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint) =0;

      virtual void create(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created,
            zoidfs_op_hint_t * hint) =0;

      virtual void remove(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint) =0;

      virtual void rename(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint) =0;

      virtual void link(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint) =0;


      virtual void symlink(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint) =0;

      virtual void mkdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint) =0;

      virtual void readdir(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint) =0;

      virtual void resize(const iofwdevent::CBType & cb, int * ret, const zoidfs_handle_t * handle,
            uint64_t size,
            zoidfs_op_hint_t * hint) =0;
   public:
      virtual ~ZoidFSAsync ();
};


//===========================================================================
} /* namespace util */

} /* namespace zoidfs */

#endif /* __ZOIDFS_UTIL_ZOIDFSASYNC_HH__ */
