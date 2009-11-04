
#include <errno.h>`
#include "zoidfs/zoidfs.h"
#include "../zint-handler.h"
#include "c-util/tools.h"

static int zint_local_null(void)
{
    return ZFS_OK;
}

static int zint_local_getattr(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_setattr(const zoidfs_handle_t * UNUSED(handle),
                       const zoidfs_sattr_t * UNUSED(sattr),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_lookup(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      zoidfs_handle_t * UNUSED(handle),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_readlink(const zoidfs_handle_t * UNUSED(handle),
                        char * UNUSED(buffer),
                        size_t UNUSED(buffer_length),
                        zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_read(const zoidfs_handle_t * UNUSED(handle),
                    size_t UNUSED(mem_count),
                    void * UNUSED(mem_starts[]),
                    const size_t UNUSED(mem_sizes[]),
                    size_t UNUSED(file_count),
                    const uint64_t UNUSED(file_starts[]),
                    uint64_t UNUSED(file_sizes[]),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_write(const zoidfs_handle_t * UNUSED(handle),
                     size_t UNUSED(mem_count),
                     const void * UNUSED(mem_starts[]),
                     const size_t UNUSED(mem_sizes[]),
                     size_t UNUSED(file_count),
                     const uint64_t UNUSED(file_starts[]),
                     uint64_t UNUSED(file_sizes[]),
                     zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_commit(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_create(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      const zoidfs_sattr_t * UNUSED(attr),
                      zoidfs_handle_t * UNUSED(handle),
                      int * UNUSED(created),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_remove(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_rename(const zoidfs_handle_t * UNUSED(from_parent_handle),
                      const char * UNUSED(from_component_name),
                      const char * UNUSED(from_full_path),
                      const zoidfs_handle_t * UNUSED(to_parent_handle),
                      const char * UNUSED(to_component_name),
                      const char * UNUSED(to_full_path),
                      zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                      zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_link(const zoidfs_handle_t * UNUSED(from_parent_handle),
                    const char * UNUSED(from_component_name),
                    const char * UNUSED(from_full_path),
                    const zoidfs_handle_t * UNUSED(to_parent_handle),
                    const char * UNUSED(to_component_name),
                    const char * UNUSED(to_full_path),
                    zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                    zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_symlink(    
    const zoidfs_handle_t * UNUSED(from_parent_handle),
    const char * UNUSED(from_component_name),
    const char * UNUSED(from_full_path),
    const zoidfs_handle_t * UNUSED(to_parent_handle),
    const char * UNUSED(to_component_name),
    const char * UNUSED(to_full_path),
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(from_parent_hint),
    zoidfs_cache_hint_t * UNUSED(to_parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_mkdir(
    const zoidfs_handle_t * UNUSED(parent_handle),
    const char * UNUSED(component_name),
    const char * UNUSED(full_path),
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_readdir(const zoidfs_handle_t * UNUSED(parent_handle),
                       zoidfs_dirent_cookie_t UNUSED(cookie),
                       size_t * UNUSED(entry_count),
                       zoidfs_dirent_t * UNUSED(entries),
                       uint32_t UNUSED(flags),
                       zoidfs_cache_hint_t * UNUSED(parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_resize(const zoidfs_handle_t * UNUSED(handle),
                      uint64_t UNUSED(size),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zint_local_resolve_path(const char * UNUSED(local_path),
                            char * UNUSED(fs_path),
                            int UNUSED(fs_path_max),
                            zoidfs_handle_t * UNUSED(newhandle),
                            int * UNUSED(usenew),
                            zoidfs_op_hint_t * UNUSED(op_hint))
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
