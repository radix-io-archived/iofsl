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

/* dispatcher variables */
//static void * nofs_handle_tree_root = NULL;

/* based on http://burtleburtle.net/bob/hash/doobs.html */
static size_t zoidfs_nofs_handle_create(const zoidfs_handle_t * parent_handle, const char * component_name, const char * full_path, zoidfs_handle_t * handle)
{
    size_t hash = 0;
    size_t i = 0;


    /* if the right vars are not set, just return 0 */
    if(!(full_path || (parent_handle && component_name)))
    {
        return 0;
    }

    /* if the fullpath is available */
    if(full_path)
    {
        /* cycle through the handle data field */
        for(i = 0 ; i < strlen(full_path) ; i++)
        {
            hash += full_path[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }

        /* final bit manipulations */
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);

        /* assign the hash to the handle */
        if(handle)
        {
            *(size_t *)(&handle->data[0]) = hash;
        }

        return hash;
    }
    /*else, use the component_name and parent handle */
    else
    {
        hash += *(size_t *)(&parent_handle->data[0]);
        hash += (hash << 10);
        hash ^= (hash >> 6);

        /* cycle through the handle data field */
        for(i = 0 ; i < strlen(component_name) ; i++)
        {
            hash += component_name[i];
            hash += (hash << 10);
            hash ^= (hash >> 6);
        }

        /* final bit manipulations */
        hash += (hash << 3);
        hash ^= (hash >> 11);
        hash += (hash << 15);

        /* assign the hash to the handle */
        if(handle)
        {
            *(size_t *)(&handle->data[0]) = hash;
        }

        return hash;
    }
    return 0;
}

/* compare the handle hashes */
/*static int zoidfs_nofs_handle_compare(const void * a, const void * b)
{
    if(*(size_t *)a > *(size_t *)b)
    {
        return 1;
    }
    if(*(size_t *)a > *(size_t *)b)
    {
        return -1;
    }
    return 0;
}*/

/* comment out the hand cache tree for now ... */
#if 0
/* add a handle to the tree */
static int zoidfs_nofs_handle_register(size_t handle)
{

    if(tfind(&handle, &nofs_handle_tree_root, zoidfs_nofs_handle_compare) == NULL)
    {
        size_t * h = (size_t *)malloc(sizeof(size_t));
        *h = handle;
        tsearch(h, &nofs_handle_tree_root, zoidfs_nofs_handle_compare);
    }

    return 0;
}

static int zoidfs_nofs_handle_exist(zoidfs_handle_t * parent_handle, char * component_name, char * full_path)
{
    size_t h = zoidfs_nofs_handle_create(parent_handle, component_name, full_path, NULL);

    if(tfind(&h, &nofs_handle_tree_root, zoidfs_nofs_handle_compare) != NULL)
    {
        return ZFS_OK;
    }
    return ZFSERR_OTHER;
}

/* remove a handle from the tree */
static int zoidfs_nofs_handle_unregister(size_t handle)
{
    tdelete(&handle, &nofs_handle_tree_root, zoidfs_nofs_handle_compare);
    return 0;
}

static int zoidfs_nofs_handle_cleanup()
{
    tdestroy(nofs_handle_tree_root, free);
    return 0;
}
#endif

static int zoidfs_nofs_null(void)
{
    return ZFS_OK;
}

static int zoidfs_nofs_getattr(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_setattr(const zoidfs_handle_t * UNUSED(handle),
                       const zoidfs_sattr_t * UNUSED(sattr),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_lookup(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_handle_t * handle,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    zoidfs_nofs_handle_create(parent_handle, component_name, full_path, handle);
    return ZFS_OK;
}

static int zoidfs_nofs_readlink(const zoidfs_handle_t * UNUSED(handle),
                        char * UNUSED(buffer),
                        size_t UNUSED(buffer_length),
                        zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_read(const zoidfs_handle_t * UNUSED(handle),
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

static int zoidfs_nofs_write(const zoidfs_handle_t * UNUSED(handle),
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

static int zoidfs_nofs_commit(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_create(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      const zoidfs_sattr_t * UNUSED(attr),
                      zoidfs_handle_t * handle,
                      int * UNUSED(created),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    zoidfs_nofs_handle_create(parent_handle, component_name, full_path, handle);
    return ZFS_OK;
}

static int zoidfs_nofs_remove(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_rename(const zoidfs_handle_t * UNUSED(from_parent_handle),
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

static int zoidfs_nofs_link(const zoidfs_handle_t * UNUSED(from_parent_handle),
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

static int zoidfs_nofs_symlink(
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

static int zoidfs_nofs_mkdir(
    const zoidfs_handle_t * UNUSED(parent_handle),
    const char * UNUSED(component_name),
    const char * UNUSED(full_path),
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_readdir(const zoidfs_handle_t * UNUSED(parent_handle),
                       zoidfs_dirent_cookie_t UNUSED(cookie),
                       size_t * UNUSED(entry_count),
                       zoidfs_dirent_t * UNUSED(entries),
                       uint32_t UNUSED(flags),
                       zoidfs_cache_hint_t * UNUSED(parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_resize(const zoidfs_handle_t * UNUSED(handle),
                      uint64_t UNUSED(size),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_nofs_resolve_path(const char * nofs_path,
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
        strncpy(fs_path, nofs_path, fs_path_max);
        fs_path[fs_path_max - 1] = '\0';
    }

    return ZFS_OK;
}

static int zoidfs_nofs_init(void)
{
    return ZFS_OK;
}

static int zoidfs_nofs_finalize(void)
{
    return ZFS_OK;
}

static int zoidfs_nofs_set_options(ConfigHandle UNUSED(c), SectionHandle UNUSED(s))
{
    return ZFS_OK;
}

zint_handler_t nofs_handler =
{
    zoidfs_nofs_null,
    zoidfs_nofs_getattr,
    zoidfs_nofs_setattr,
    zoidfs_nofs_lookup,
    zoidfs_nofs_readlink,
    zoidfs_nofs_read,
    zoidfs_nofs_write,
    zoidfs_nofs_commit,
    zoidfs_nofs_create,
    zoidfs_nofs_remove,
    zoidfs_nofs_rename,
    zoidfs_nofs_link,
    zoidfs_nofs_symlink,
    zoidfs_nofs_mkdir,
    zoidfs_nofs_readdir,
    zoidfs_nofs_resize,
    zoidfs_nofs_resolve_path,
    zoidfs_nofs_init,
    zoidfs_nofs_finalize,
    zoidfs_nofs_set_options
};
