#define _GNU_SOURCE
#include <search.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zoidfs/zoidfs.h"
#include "zoidfs/dispatcher/zint-handler.h"
#include "c-util/tools.h"
#include "c-util/configfile.h"

static int zoidfs_ross_null(void)
{
    return ZFS_OK;
}

static int zoidfs_ross_getattr(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_setattr(const zoidfs_handle_t * UNUSED(handle),
                       const zoidfs_sattr_t * UNUSED(sattr),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_lookup(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      zoidfs_handle_t * UNUSED(handle),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_readlink(const zoidfs_handle_t * UNUSED(handle),
                        char * UNUSED(buffer),
                        size_t UNUSED(buffer_length),
                        zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_read(const zoidfs_handle_t * UNUSED(handle),
                    size_t UNUSED(mem_count),
                    void * UNUSED(mem_starts[]),
                    const size_t UNUSED(mem_sizes[]),
                    size_t UNUSED(file_count),
                    const zoidfs_file_ofs_t UNUSED(file_starts[]),
                    zoidfs_file_size_t UNUSED(file_sizes[]),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_write(const zoidfs_handle_t * UNUSED(handle),
                     size_t UNUSED(mem_count),
                     const void * UNUSED(mem_starts[]),
                     const size_t UNUSED(mem_sizes[]),
                     size_t UNUSED(file_count),
                     const zoidfs_file_ofs_t UNUSED(file_starts[]),
                     zoidfs_file_size_t UNUSED(file_sizes[]),
                     zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_commit(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_create(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      const zoidfs_sattr_t * UNUSED(attr),
                      zoidfs_handle_t * UNUSED(handle),
                      int * UNUSED(created),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    zoidfs_ross_handle_create(parent_handle, component_name, full_path, handle);
    return ZFS_OK;
}

static int zoidfs_ross_remove(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_rename(const zoidfs_handle_t * UNUSED(from_parent_handle),
                      const char * UNUSED(from_component_name),
                      const char * UNUSED(from_full_path),
                      const zoidfs_handle_t * UNUSED(to_parent_handle),
                      const char * UNUSED(to_component_name),
                      const char * UNUSED(to_full_path),
                      zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                      zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_link(const zoidfs_handle_t * UNUSED(from_parent_handle),
                    const char * UNUSED(from_component_name),
                    const char * UNUSED(from_full_path),
                    const zoidfs_handle_t * UNUSED(to_parent_handle),
                    const char * UNUSED(to_component_name),
                    const char * UNUSED(to_full_path),
                    zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                    zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_symlink(
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
    return ZFS_OK;
}

static int zoidfs_ross_mkdir(
    const zoidfs_handle_t * UNUSED(parent_handle),
    const char * UNUSED(component_name),
    const char * UNUSED(full_path),
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_readdir(const zoidfs_handle_t * UNUSED(parent_handle),
                       zoidfs_dirent_cookie_t UNUSED(cookie),
                       size_t * UNUSED(entry_count),
                       zoidfs_dirent_t * UNUSED(entries),
                       uint32_t UNUSED(flags),
                       zoidfs_cache_hint_t * UNUSED(parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_resize(const zoidfs_handle_t * UNUSED(handle),
                      uint64_t UNUSED(size),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_ross_resolve_path(const char * ross_path,
                            char * fs_path,
                            int fs_path_max,
                            zoidfs_handle_t * UNUSED(newhandle),
                            int * usenew)
{
    *usenew = 0;

    /*
     * If the fs_path buffer is available, identify the full path
     */
    if (fs_path)
    {
        strncpy(fs_path, ross_path, fs_path_max);
        fs_path[fs_path_max - 1] = '\0';
    }

    return ZFS_OK;
}

static int zoidfs_ross_init(void)
{
    return ZFS_OK;
}

static int zoidfs_ross_finalize(void)
{
    return ZFS_OK;
}

static int zoidfs_ross_set_options(ConfigHandle UNUSED(c), SectionHandle UNUSED(s))
{
    return ZFS_OK;
}

zint_handler_t ross_handler =
{
    zoidfs_ross_null,
    zoidfs_ross_getattr,
    zoidfs_ross_setattr,
    zoidfs_ross_lookup,
    zoidfs_ross_readlink,
    zoidfs_ross_read,
    zoidfs_ross_write,
    zoidfs_ross_commit,
    zoidfs_ross_create,
    zoidfs_ross_remove,
    zoidfs_ross_rename,
    zoidfs_ross_link,
    zoidfs_ross_symlink,
    zoidfs_ross_mkdir,
    zoidfs_ross_readdir,
    zoidfs_ross_resize,
    zoidfs_ross_resolve_path,
    zoidfs_ross_init,
    zoidfs_ross_finalize,
    zoidfs_ross_set_options
};
