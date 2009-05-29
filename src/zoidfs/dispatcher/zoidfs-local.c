
#include <errno.h>
#include "zoidfs/zoidfs.h"
#include "zint-handler.h"

static int zint_local_null(void)
{
    return ZFS_OK;
}

static int zint_local_getattr(const zoidfs_handle_t * handle,
                       zoidfs_attr_t * attr)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_setattr(const zoidfs_handle_t * handle,
                       const zoidfs_sattr_t * sattr,
                       zoidfs_attr_t * attr)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_lookup(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_handle_t * handle)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_readlink(const zoidfs_handle_t * handle,
                        char * buffer,
                        size_t buffer_length)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_read(const zoidfs_handle_t * handle,
                    size_t mem_count,
                    void * mem_starts[],
                    const size_t mem_sizes[],
                    size_t file_count,
                    const uint64_t file_starts[],
                    uint64_t file_sizes[])
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_write(const zoidfs_handle_t * handle,
                     size_t mem_count,
                     const void * mem_starts[],
                     const size_t mem_sizes[],
                     size_t file_count,
                     const uint64_t file_starts[],
                     uint64_t file_sizes[])
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_commit(const zoidfs_handle_t * handle)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_create(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      const zoidfs_sattr_t * attr,
                      zoidfs_handle_t * handle,
                      int * created)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_remove(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_cache_hint_t * parent_hint)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_rename(const zoidfs_handle_t * from_parent_handle,
                      const char * from_component_name,
                      const char * from_full_path,
                      const zoidfs_handle_t * to_parent_handle,
                      const char * to_component_name,
                      const char * to_full_path,
                      zoidfs_cache_hint_t * from_parent_hint,
                      zoidfs_cache_hint_t * to_parent_hint)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_link(const zoidfs_handle_t * from_parent_handle,
                    const char * from_component_name,
                    const char * from_full_path,
                    const zoidfs_handle_t * to_parent_handle,
                    const char * to_component_name,
                    const char * to_full_path,
                    zoidfs_cache_hint_t * from_parent_hint,
                    zoidfs_cache_hint_t * to_parent_hint)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_symlink(    
    const zoidfs_handle_t * from_parent_handle,
    const char * from_component_name,
    const char * from_full_path,
    const zoidfs_handle_t * to_parent_handle,
    const char * to_component_name,
    const char * to_full_path,
    const zoidfs_sattr_t * attr,
    zoidfs_cache_hint_t * from_parent_hint,
    zoidfs_cache_hint_t * to_parent_hint)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_mkdir(
    const zoidfs_handle_t * parent_handle,
    const char * component_name,
    const char * full_path,
    const zoidfs_sattr_t * attr,
    zoidfs_cache_hint_t * parent_hint)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_readdir(const zoidfs_handle_t * parent_handle,
                       zoidfs_dirent_cookie_t cookie,
                       size_t * entry_count,
                       zoidfs_dirent_t * entries,
                       uint32_t flags,
                       zoidfs_cache_hint_t * parent_hint)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_resize(const zoidfs_handle_t * handle,
                      uint64_t size)
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_resolve_path(const char * local_path,
                            char * fs_path,
                            int fs_path_max)
{
    return ZFSERR_NOENT;
}

static int zint_local_init(void)
{
    return ZFS_OK;
}

static int zint_local_finalize(void)
{
    return ZFS_OK;
}

zint_handler_t local_handler =
{
    zint_local_null,
    zint_local_getattr,
    zint_local_setattr,
    zint_local_lookup,
    zint_local_readlink,
    zint_local_read,
    zint_local_write,
    zint_local_commit,
    zint_local_create,
    zint_local_remove,
    zint_local_rename,
    zint_local_link,
    zint_local_symlink,
    zint_local_mkdir,
    zint_local_readdir,
    zint_local_resize,
    zint_local_resolve_path,
    zint_local_init,
    zint_local_finalize
};

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End: 
 *
 * vim: ts=8 sts=4 sw=4 expandtab
 */
