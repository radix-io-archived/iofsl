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

#include "globus_ftp_client.h"

/* globus locks */
static globus_mutex_t resize_lock;

/* globus condition variables */
static globus_cond_t resize_cond;

/* globus bool variables */
static globus_bool_t resize_done;
static globus_bool_t resize_success;

/* gridftp callbacks */
void resize_cb(void *myargs, globus_ftp_client_handle_t *handle,
                    globus_object_t *error)
{
    /* if error, indicate failure */
    if (error)
    {
        globus_mutex_lock(&resize_lock);
        resize_success=GLOBUS_FALSE;
        globus_mutex_unlock(&resize_lock);
    }
    /* else, inidcate success */
    else
    {
        globus_mutex_lock(&resize_lock);
        resize_success=GLOBUS_TRUE;
        globus_mutex_unlock(&resize_lock);
    }
    
    /* inidcate the operation is done */
    globus_mutex_lock(&resize_lock);
    resize_done=GLOBUS_TRUE;
    globus_cond_signal(&resize_cond);
    globus_mutex_unlock(&resize_lock);
}

static int zoidfs_gridftp_null(void)
{
    return ZFS_OK;
}

static int zoidfs_gridftp_getattr(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_setattr(const zoidfs_handle_t * UNUSED(handle),
                       const zoidfs_sattr_t * UNUSED(sattr),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_lookup(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_handle_t * handle,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_readlink(const zoidfs_handle_t * UNUSED(handle),
                        char * UNUSED(buffer),
                        size_t UNUSED(buffer_length),
                        zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_read(const zoidfs_handle_t * UNUSED(handle),
                    size_t UNUSED(mem_count),
                    void * UNUSED(mem_starts[]),
                    const size_t UNUSED(mem_sizes[]),
                    size_t UNUSED(file_count),
                    const uint64_t UNUSED(file_starts[]),
                    uint64_t UNUSED(file_sizes[]),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_write(const zoidfs_handle_t * UNUSED(handle),
                     size_t UNUSED(mem_count),
                     const void * UNUSED(mem_starts[]),
                     const size_t UNUSED(mem_sizes[]),
                     size_t UNUSED(file_count),
                     const uint64_t UNUSED(file_starts[]),
                     uint64_t UNUSED(file_sizes[]),
                     zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_commit(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_create(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      const zoidfs_sattr_t * UNUSED(attr),
                      zoidfs_handle_t * handle,
                      int * UNUSED(created),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_remove(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_rename(const zoidfs_handle_t * UNUSED(from_parent_handle),
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

static int zoidfs_gridftp_link(const zoidfs_handle_t * UNUSED(from_parent_handle),
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

static int zoidfs_gridftp_symlink(
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

static int zoidfs_gridftp_mkdir(
    const zoidfs_handle_t * UNUSED(parent_handle),
    const char * UNUSED(component_name),
    const char * UNUSED(full_path),
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_readdir(const zoidfs_handle_t * UNUSED(parent_handle),
                       zoidfs_dirent_cookie_t UNUSED(cookie),
                       size_t * UNUSED(entry_count),
                       zoidfs_dirent_t * UNUSED(entries),
                       uint32_t UNUSED(flags),
                       zoidfs_cache_hint_t * UNUSED(parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_resize(const zoidfs_handle_t * UNUSED(handle),
                      uint64_t UNUSED(size),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_resolve_path(const char * gridftp_path,
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
        strncpy(fs_path, gridftp_path, fs_path_max);
        fs_path[fs_path_max - 1] = '\0';
    }

    return ZFS_OK;
}

static int zoidfs_gridftp_init(void)
{
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    return ZFS_OK;
}

static int zoidfs_gridftp_finalize(void)
{
    globus_module_deactivate(GLOBUS_FTP_CLIENT_MODULE);
    return ZFS_OK;
}

static int zoidfs_gridftp_set_options(ConfigHandle UNUSED(c), SectionHandle UNUSED(s))
{
    return ZFS_OK;
}

zint_handler_t gridftp_handler =
{
    zoidfs_gridftp_null,
    zoidfs_gridftp_getattr,
    zoidfs_gridftp_setattr,
    zoidfs_gridftp_lookup,
    zoidfs_gridftp_readlink,
    zoidfs_gridftp_read,
    zoidfs_gridftp_write,
    zoidfs_gridftp_commit,
    zoidfs_gridftp_create,
    zoidfs_gridftp_remove,
    zoidfs_gridftp_rename,
    zoidfs_gridftp_link,
    zoidfs_gridftp_symlink,
    zoidfs_gridftp_mkdir,
    zoidfs_gridftp_readdir,
    zoidfs_gridftp_resize,
    zoidfs_gridftp_resolve_path,
    zoidfs_gridftp_init,
    zoidfs_gridftp_finalize,
    zoidfs_gridftp_set_options
};
