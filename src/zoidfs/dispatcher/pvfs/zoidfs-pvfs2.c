/*
 * zoidfs-pvfs2.c
 * PVFS driver for ZOIDFS.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include "zoidfs/zoidfs.h"
#include "../zint-handler.h"
#include "c-util/tools.h"

#include "pvfs2-sysint.h"
#include "pvfs2-util.h"

#include "pvfs2.h"

#define zint_pvfs2_input_handle(_zfs_handle, _pvfs2_ref) \
    do { \
        uint8_t * _zptr = (uint8_t *)(_zfs_handle)->data + sizeof(uint32_t); \
        (_pvfs2_ref).handle = *((PVFS_handle *) _zptr); \
        _zptr += sizeof(PVFS_handle); \
        (_pvfs2_ref).fs_id = *((PVFS_fs_id *) _zptr); \
    } while(0)

#define zint_pvfs2_output_handle(_pvfs2_ref, _zfs_handle) \
    do { \
        uint8_t * _zptr = (_zfs_handle)->data; \
        memset(_zptr, 0, 32); \
        _zptr += sizeof(uint32_t); /* reserved for zoidfs */ \
        *(PVFS_handle *) _zptr = (_pvfs2_ref).handle; \
        _zptr += sizeof(PVFS_handle); \
        *(PVFS_fs_id *) _zptr = (_pvfs2_ref).fs_id; \
    } while(0)

#define zint_pvfs2_input_time(_ztime) \
    ((_ztime).seconds)

#define zint_pvfs2_output_time(_ptime, _ztime) \
    do { \
        (_ztime).seconds = _ptime; \
        (_ztime).nseconds = 0; \
    } while(0)

#define zint_pvfs2_input_creds(_sattr, _creds) \
    (_creds).uid = (_sattr)->uid; (_creds).gid = (_sattr)->gid



inline static uint32_t zint_pvfs2_input_attr_mask(uint16_t zmask);

inline static uint16_t zint_pvfs2_output_attr_mask(uint32_t pmask);

inline static int zint_pvfs2_input_sattr(const zoidfs_sattr_t * zsattr,
                                         PVFS_sys_attr * resattr);

inline static PVFS_ds_type zint_pvfs2_input_attr_type(zoidfs_attr_type_t
                                                      ztype);

inline static int zint_pvfs2_input_attr(const zoidfs_attr_t * zattr,
                                        PVFS_credentials creds,
                                        PVFS_sys_attr * resattr);

inline static int zint_pvfs2_output_attr(PVFS_sys_attr attr,
                                         zoidfs_attr_t * zattr);

inline static int zint_pvfs2_output_error(PVFS_error error);

inline static void zint_pvfs2_split_path(const char * path,
                                         char ** base,
                                         char ** component);

inline static void zint_pvfs2_pattr_fill_setable(PVFS_sys_attr * pattr,
                                                 uint16_t mode);

inline static uint32_t zint_pvfs2_input_attr_mask(uint16_t zmask)
{
    uint32_t pmask = 0;

    if(zmask & ZOIDFS_ATTR_MODE)
    {
        pmask |= PVFS_ATTR_SYS_PERM;
    }

    if(zmask & ZOIDFS_ATTR_UID)
    {
        pmask |= PVFS_ATTR_SYS_UID;
    }

    if(zmask & ZOIDFS_ATTR_GID)
    {
        pmask |= PVFS_ATTR_SYS_GID;
    }

    if(zmask & ZOIDFS_ATTR_SIZE)
    {
        pmask |= PVFS_ATTR_SYS_SIZE;
    }

    if(zmask & ZOIDFS_ATTR_ATIME)
    {
        pmask |= PVFS_ATTR_SYS_ATIME;
    }

    if(zmask & ZOIDFS_ATTR_CTIME)
    {
        pmask |= PVFS_ATTR_SYS_CTIME;
    }

    if(zmask & ZOIDFS_ATTR_MTIME)
    {
        pmask |= PVFS_ATTR_SYS_MTIME;
    }

    return pmask;
}

inline static uint16_t zint_pvfs2_output_attr_mask(uint32_t pmask)
{
    uint16_t zmask = 0;

    if(pmask & PVFS_ATTR_SYS_PERM)
    {
        pmask |= ZOIDFS_ATTR_MODE;
    }

    if(pmask & PVFS_ATTR_SYS_UID)
    {
        zmask |= ZOIDFS_ATTR_UID;
    }

    if(pmask & PVFS_ATTR_SYS_GID)
    {
        zmask |= ZOIDFS_ATTR_GID;
    }

    if(pmask & PVFS_ATTR_SYS_SIZE)
    {
        zmask |= ZOIDFS_ATTR_SIZE;
    }

    if(pmask & PVFS_ATTR_SYS_ATIME)
    {
        zmask |= ZOIDFS_ATTR_ATIME;
    }

    if(pmask & PVFS_ATTR_SYS_CTIME)
    {
        zmask |= ZOIDFS_ATTR_CTIME;
    }

    if(pmask & PVFS_ATTR_SYS_MTIME)
    {
        zmask |= ZOIDFS_ATTR_MTIME;
    }

    return zmask;
}

inline static PVFS_ds_type zint_pvfs2_input_attr_type(zoidfs_attr_type_t ztype)
{
    if(ztype == ZOIDFS_REG)
    {
        return PVFS_TYPE_METAFILE;
    }
    else if(ztype == ZOIDFS_DIR)
    {
        return PVFS_TYPE_DIRECTORY;
    }
    else if(ztype == ZOIDFS_LNK)
    {
        return PVFS_TYPE_SYMLINK;
    }
    return -1;
}

inline static zoidfs_attr_type_t zint_pvfs2_output_attr_type(PVFS_ds_type
                                                             ptype)
{
    if(ptype == PVFS_TYPE_METAFILE)
    {
        return ZOIDFS_REG;
    }
    else if(ptype == PVFS_TYPE_DIRECTORY)
    {
        return ZOIDFS_DIR;
    }
    else if(ptype == PVFS_TYPE_SYMLINK)
    {
        return ZOIDFS_LNK;
    }
    return ZOIDFS_INVAL;
}

inline static int zint_pvfs2_input_sattr(const zoidfs_sattr_t * zsattr,
                                         PVFS_sys_attr * resattr)
{
    PVFS_sys_attr pattr;

    memset(&pattr, 0, sizeof(PVFS_sys_attr));
    pattr.owner = zsattr->uid;
    pattr.group = zsattr->gid;
    pattr.perms = zsattr->mode;
    pattr.atime = zint_pvfs2_input_time(zsattr->atime);
    pattr.mtime = zint_pvfs2_input_time(zsattr->mtime);
    pattr.size  = (PVFS_size) zsattr->size;
    pattr.mask = zint_pvfs2_input_attr_mask(zsattr->mask);

    *resattr = pattr;

    return ZFS_OK;
}

inline static int zint_pvfs2_input_attr(const zoidfs_attr_t * zattr,
                                        PVFS_credentials creds,
                                        PVFS_sys_attr * pattr)
{
    pattr->owner = creds.uid;
    pattr->group = creds.gid;
    pattr->perms = zattr->mode;
    pattr->atime = zint_pvfs2_input_time(zattr->atime);
    pattr->mtime = zint_pvfs2_input_time(zattr->mtime);
    pattr->size  = (PVFS_size) zattr->size;

    pattr->link_target = NULL;
    pattr->dfile_count = 0;
    pattr->dirent_count = 0;
    pattr->objtype = zint_pvfs2_input_attr_type(zattr->type);
    pattr->mask = zint_pvfs2_input_attr_mask(zattr->mask);

    return ZFS_OK;
}

inline static int zint_pvfs2_output_attr(PVFS_sys_attr attr,
                                         zoidfs_attr_t * zattr)
{
    zattr->mode = attr.perms;
    zint_pvfs2_output_time(attr.atime, zattr->atime);
    zint_pvfs2_output_time(attr.mtime, zattr->mtime);
    zattr->size = attr.size;
    zattr->uid = attr.owner;
    zattr->gid = attr.group; 
    zattr->fsid = 0;
    zattr->fileid = 0; 
    zattr->blocksize = 4*1024*1024; 
    zattr->nlink = 2; /* pvfs has no hardlinks */ 
    zattr->type = zint_pvfs2_output_attr_type(attr.objtype);
    zattr->mask = zint_pvfs2_output_attr_mask(attr.mask);

    return ZFS_OK;
}

inline static void zint_pvfs2_pattr_fill_setable(PVFS_sys_attr * pattr,
                                                 uint16_t mode)
{
    if(!(pattr->mask & PVFS_ATTR_SYS_PERM))
    {
        pattr->mask |= PVFS_ATTR_SYS_PERM;
        pattr->perms = mode;
    }
    if(!(pattr->mask & PVFS_ATTR_SYS_UID))
    {
        pattr->mask |= PVFS_ATTR_SYS_UID;
        pattr->owner = geteuid();
    }
    if(!(pattr->mask & PVFS_ATTR_SYS_GID))
    {
        pattr->mask |= PVFS_ATTR_SYS_GID;
        pattr->group = getegid();
    }
    if(!(pattr->mask & PVFS_ATTR_SYS_MTIME))
    {
        pattr->mask |= PVFS_ATTR_SYS_MTIME;
        pattr->mtime = time(NULL);
    }
    if(!(pattr->mask & PVFS_ATTR_SYS_ATIME))
    {
        pattr->mask |= PVFS_ATTR_SYS_ATIME;
        pattr->atime = pattr->mtime;
    }
    if(!(pattr->mask & PVFS_ATTR_SYS_CTIME))
    {
        pattr->mask |= PVFS_ATTR_SYS_CTIME;
        pattr->ctime = pattr->mtime;
    }
}

inline static int zint_pvfs2_output_error(PVFS_error error)
{
    switch(-error)
    {
    case PVFS_EPERM:
        return ZFSERR_PERM;
    case PVFS_ENOENT:
        return ZFSERR_NOENT;
    case PVFS_EIO:
        return ZFSERR_IO;
    case PVFS_ENXIO:
        return ZFSERR_NXIO;
    case PVFS_EACCES:
        return ZFSERR_ACCES;
    case PVFS_EEXIST:
        return ZFSERR_EXIST;
    case PVFS_ENODEV:
        return ZFSERR_NODEV;
    case PVFS_ENOTDIR:
        return ZFSERR_NOTDIR;
    case PVFS_EISDIR:
        return ZFSERR_ISDIR;
    case PVFS_EFBIG:
        return ZFSERR_FBIG;
    case PVFS_ENOSPC:
        return ZFSERR_NOSPC;
    case PVFS_EROFS:
        return ZFSERR_ROFS;
    case PVFS_ENAMETOOLONG:
        return ZFSERR_NAMETOOLONG;
    case PVFS_ENOTEMPTY:
        return ZFSERR_NOTEMPTY;
    }

    return -error;
}

inline static void zint_pvfs2_split_path(const char * path,
                                         char ** base,
                                         char ** component)
{
    char * tmpstr;
    char * index;

    assert(path);

    tmpstr = strdup(path);

    index = rindex(tmpstr, '/');
    if(index)
    {
        *index = 0;
        ++index;

        *component = strdup(index);
        *base = tmpstr;
        /* The condition below will match for path such as "/file"
           (only one "/", right in front).  We make sure not to return
           an empty base in such a case.  */
        if (strlen(*base) == 0)
            strcpy(*base, "/");
    }
    else
    {
        *component = tmpstr;
        *base = NULL;
    }
}

static int zint_pvfs2_null(void)
{
   /* need to ping pvfs server here */ 
    return ZFS_OK;
}

static int zint_pvfs2_lookup(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_handle_t * handle,
                      zoidfs_op_hint_t * op_hint);

static int zint_pvfs2_create(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      const zoidfs_sattr_t * sattr,
                      zoidfs_handle_t * handle,
                      int * created,
                      zoidfs_op_hint_t * op_hint)
{
    int ret;
    PVFS_object_ref ref;
    PVFS_sys_attr pattr;
    PVFS_credentials creds;
    PVFS_sysresp_create create_resp;
    PVFS_sysresp_lookup lookup_resp;

    PVFS_util_gen_credentials(&creds);

    ret = zint_pvfs2_input_sattr(sattr, &pattr);
    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    zint_pvfs2_pattr_fill_setable(&pattr, 0644);

    zint_pvfs2_input_handle(parent_handle, ref);

    if(component_name && parent_handle)
    {
        /* use the default distribution */
        ret = PVFS_sys_create((char *)component_name, ref, pattr, &creds,
                              NULL, &create_resp, NULL, NULL);
    }
    else if(full_path)
    {
        char * component_str;
        char * base_str;

        zint_pvfs2_split_path(full_path, &base_str, &component_str);

        ret = PVFS_sys_lookup(ref.fs_id, base_str, &creds, &lookup_resp,
                              PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);

        if(base_str)
        {
            free(base_str);
        }

        if(ret < 0)
        {
            free(component_str);

            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_create((char *)component_str, lookup_resp.ref, pattr,
                              &creds, NULL, &create_resp, NULL, NULL);

        free(component_str);
    }

    if(ret < 0)
    {
        if(ret == -PVFS_EEXIST)
        {
            *created = 0;

            return zint_pvfs2_lookup(parent_handle, component_name,
                                     full_path, handle, op_hint);
        }
        else
        {
            return zint_pvfs2_output_error(ret);
        }
    }
    else
    {
        *created = 1;
    }

    zint_pvfs2_output_handle(create_resp.ref, handle);

    return ZFS_OK;
}

static int zint_pvfs2_getattr(const zoidfs_handle_t * handle, zoidfs_attr_t * attr,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_credentials creds;
    PVFS_object_ref ref;
    PVFS_sysresp_getattr getattr_resp;
    uint32_t mask;

    zint_pvfs2_input_handle(handle, ref);

    PVFS_util_gen_credentials(&creds);

    mask = zint_pvfs2_input_attr_mask(attr->mask);

    ret = PVFS_sys_getattr(ref, mask, &creds, &getattr_resp, NULL);
    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    zint_pvfs2_output_attr(getattr_resp.attr, attr);

    return ZFS_OK;
}

static int zint_pvfs2_setattr(const zoidfs_handle_t * handle,
                       const zoidfs_sattr_t * sattr,
                       zoidfs_attr_t * attr,
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_credentials creds;
    PVFS_object_ref ref;
    PVFS_sys_attr pattr;
    PVFS_sysresp_getattr getattr_resp;
    uint32_t mask;

    zint_pvfs2_input_handle(handle, ref);

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_sattr(sattr, &pattr);

    ret = PVFS_sys_setattr(ref, pattr, &creds, NULL);
    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    if(attr)
    {
        mask = zint_pvfs2_input_attr_mask(attr->mask);

        ret = PVFS_sys_getattr(ref, mask, &creds, &getattr_resp, NULL);
        if(ret < 0)
        {
            return zint_pvfs2_output_error(ret);
        }

        zint_pvfs2_output_attr(getattr_resp.attr, attr);
    }

    return ZFS_OK;
}

static int zint_pvfs2_lookup(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_handle_t * handle,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int res;
    PVFS_object_ref ref;
    PVFS_credentials creds;
    PVFS_sysresp_lookup lookup_resp;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_handle(parent_handle, ref);

    if(component_name)
    {
        res = PVFS_sys_ref_lookup(ref.fs_id, (char *)component_name,
                                  ref, &creds, &lookup_resp,
                                  PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);
    }
    else
    {
        res = PVFS_sys_lookup(ref.fs_id, (char *)full_path,
                              &creds, &lookup_resp,
                              PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);
    }

    if(res < 0)
    {
        return zint_pvfs2_output_error(res);
    }

    zint_pvfs2_output_handle(lookup_resp.ref, handle);
    return ZFS_OK;
}

static int zint_pvfs2_readlink(const zoidfs_handle_t * handle,
                        char * buffer,
                        size_t buffer_length,
                        zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    uint32_t mask;
    PVFS_object_ref ref;
    PVFS_credentials creds;
    PVFS_sysresp_getattr getattr_resp;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_handle(handle, ref);
    mask = zint_pvfs2_input_attr_mask(ZOIDFS_ATTR_ALL);

    ret = PVFS_sys_getattr(ref, mask, &creds, &getattr_resp, NULL);
    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    strncpy(buffer, getattr_resp.attr.link_target, buffer_length);

    return ZFS_OK;
}

static int zint_pvfs2_io(const zoidfs_handle_t * handle,
                         size_t mem_count,
                         const void * mem_starts[],
                         const size_t mem_sizes[],
                         size_t file_count,
                         const uint64_t file_starts[],
                         uint64_t file_sizes[],
                         enum PVFS_io_type type,
                         PVFS_sysresp_io *io_resp,
                         zoidfs_op_hint_t * op_hint);

static int zint_pvfs2_write(const zoidfs_handle_t * handle,
                     size_t mem_count,
                     const void * mem_starts[],
                     const size_t mem_sizes[],
                     size_t file_count,
                     const uint64_t file_starts[],
                     uint64_t file_sizes[],
                     zoidfs_op_hint_t * op_hint)
{
    int ret;
    PVFS_sysresp_io write_resp;

    ret = zint_pvfs2_io(handle, mem_count, mem_starts, mem_sizes,
                        file_count, file_starts, file_sizes,
                        PVFS_IO_WRITE, &write_resp, op_hint);
    return ret;
}

static int zint_pvfs2_read(const zoidfs_handle_t * handle,
                    size_t mem_count,
                    void * mem_starts[],
                    const size_t mem_sizes[],
                    size_t file_count,
                    const uint64_t file_starts[],
                    uint64_t file_sizes[],
                    zoidfs_op_hint_t * op_hint)
{
    int ret;
    size_t total;
    unsigned int index;
    PVFS_sysresp_io read_resp;

    ret = zint_pvfs2_io(handle, mem_count,
                        (const void **)mem_starts, mem_sizes,
                        file_count, file_starts, file_sizes,
                        PVFS_IO_READ, &read_resp, op_hint);
    if(ret != ZFS_OK)
    {
        return ret;
    }

    total = read_resp.total_completed;
    index = 0;
    /* fill in sizes */
    while(index < file_count && total >= file_sizes[index])
    {
        total -= file_sizes[index];
        ++index;
    }
    if(index < file_count)
    {
        file_sizes[index] = total;
    }
    while(index < file_count)
    {
        file_sizes[index] = 0;
        ++index;
    }

    return ZFS_OK;
}

static int zint_pvfs2_io(const zoidfs_handle_t * handle,
                         size_t mem_count,
                         const void * mem_starts[],
                         const size_t mem_sizes[],
                         size_t file_count,
                         const uint64_t file_starts[],
                         uint64_t file_sizes[],
                         enum PVFS_io_type type,
                         PVFS_sysresp_io *io_resp,
                         zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    unsigned int i;
    uint64_t offset = 0;
    PVFS_object_ref ref;
    PVFS_Request mem_req;
    PVFS_Request file_req;
    PVFS_size * displacements;
    PVFS_credentials creds;
    const void * buffer;
    int32_t * mem_sizes_i32 = (int32_t *)malloc(sizeof(int32_t) * mem_count);
    int32_t * file_sizes_i32 = (int32_t *)malloc(sizeof(int32_t) * file_count);

    // PVFS_Request_indexed() requests int32_t* for blocklengths, but ZFS uses
    // uint64_t* for representing sizes. So, the conversion is needed.
    for (i = 0; i < mem_count; i++)
        mem_sizes_i32[i] = mem_sizes[i];
    for (i = 0; i < file_count; i++)
        file_sizes_i32[i] = file_sizes[i];

    PVFS_util_gen_credentials(&creds);

    if(mem_count == 1)
    {
        /* contig in memory */
        ret = PVFS_Request_contiguous(mem_sizes_i32[0], PVFS_BYTE, &mem_req);
        if(ret < 0)
        {
            goto error_pvfs;
        }

        buffer = mem_starts[0];
    }
    else
    {
        displacements = malloc(sizeof(PVFS_size) * mem_count);
        if(!displacements)
        {
            ret = ZFSERR_NOMEM;
            goto error;
        }

        for(i = 0; i < mem_count; ++i)
        {
            displacements[i] = (intptr_t)mem_starts[i];
        }

        ret = PVFS_Request_indexed(mem_count, mem_sizes_i32,
                                   displacements, PVFS_BYTE, &mem_req);

        free(displacements);

        if(ret < 0)
        {
            goto error_pvfs;
        }

        buffer = PVFS_BOTTOM;
    }

    if(file_count == 1)
    {
        /* file request is contiguous */
        offset = file_starts[0];
        ret = PVFS_Request_contiguous(file_sizes_i32[0], PVFS_BYTE, &file_req);
        if(ret < 0)
        {
            goto error_pvfs;
        }
    }
    else
    {
        assert(sizeof(PVFS_size) == sizeof(uint64_t));
        ret = PVFS_Request_indexed(file_count, file_sizes_i32,
                                   (PVFS_size *)file_starts,
                                   PVFS_BYTE, &file_req);
        if(ret < 0)
        {
            goto error_pvfs;
        }
    }

    zint_pvfs2_input_handle(handle, ref);

    ret = PVFS_sys_io(ref, file_req, offset, (void *) buffer, mem_req,
                      &creds, io_resp, type, NULL);
    if(ret < 0)
    {
        goto error_pvfs;
    }

    PVFS_Request_free(&mem_req);
    PVFS_Request_free(&file_req);

    free(mem_sizes_i32);
    free(file_sizes_i32);

    return ZFS_OK;

error:
    free(mem_sizes_i32);
    free(file_sizes_i32);
    return ret;

error_pvfs:
    free(mem_sizes_i32);
    free(file_sizes_i32);
    return zint_pvfs2_output_error(ret);
}

static int zint_pvfs2_commit(const zoidfs_handle_t * handle,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_object_ref ref;
    PVFS_credentials creds;

    zint_pvfs2_input_handle(handle, ref);
    PVFS_util_gen_credentials(&creds);

    ret = PVFS_sys_flush(ref, &creds, NULL);
    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}

static int zint_pvfs2_remove(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_object_ref ref;
    PVFS_credentials creds;
    PVFS_sysresp_lookup lookup_resp;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_handle(parent_handle, ref);

    if(component_name)
    {
        ret = PVFS_sys_remove((char *)component_name, ref, &creds, NULL);
    }
    else
    {
        char * component_str;
        char * base_str;

        zint_pvfs2_split_path(full_path, &base_str, &component_str);
 
        ret = PVFS_sys_lookup(ref.fs_id, base_str, &creds, &lookup_resp,
                              PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);

        if(base_str)
        {
            free(base_str);
        }

        if(ret < 0)
        {
            free(component_str);
            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_remove(component_str, lookup_resp.ref, &creds, NULL);

        free(component_str);
    }

    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}

static int zint_pvfs2_rename(const zoidfs_handle_t * from_parent_handle,
                      const char * from_component_name,
                      const char * from_full_path,
                      const zoidfs_handle_t * to_parent_handle,
                      const char * to_component_name,
                      const char * to_full_path,
                      zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                      zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_credentials creds;
    PVFS_object_ref from_ref, to_ref;
    PVFS_sysresp_lookup from_lookup_resp, to_lookup_resp;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_handle(from_parent_handle, from_ref);
    zint_pvfs2_input_handle(to_parent_handle, to_ref);

    if (from_component_name && to_component_name)
    {
        ret = PVFS_sys_rename((char *)from_component_name, from_ref,
                              (char *)to_component_name, to_ref, &creds, NULL);
    }
    else if (from_component_name && !to_component_name)
    {
        char * to_base_str;
        char * to_component_str;

        zint_pvfs2_split_path(to_full_path, &to_base_str, &to_component_str);

        ret = PVFS_sys_lookup(to_ref.fs_id, to_base_str, &creds,
                              &to_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                              NULL);
        if(to_base_str)
        {
            free(to_base_str);
        }

        if(ret < 0)
        {
            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_rename((char *)from_component_name, from_ref,
                              (char *)to_component_str, to_lookup_resp.ref,
                              &creds, NULL);
        free(to_component_str);
    }
    else if (!from_component_name && to_component_name)
    {
        char * from_base_str;
        char * from_component_str;

        zint_pvfs2_split_path(from_full_path, &from_base_str,
                              &from_component_str);

        ret = PVFS_sys_lookup(from_ref.fs_id, from_base_str, &creds,
                              &from_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                              NULL);
        if(from_base_str)
        {
            free(from_base_str);
        }
        if(ret < 0)
        {
            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_rename((char *)from_component_str, from_lookup_resp.ref,
                              (char *)to_component_name, to_ref, &creds, NULL);
        free(from_component_str);
    }
    else
    {
        char * from_base_str, * to_base_str;
        char * from_component_str, * to_component_str;

        zint_pvfs2_split_path(from_full_path, &from_base_str,
                              &from_component_str);
        zint_pvfs2_split_path(to_full_path, &to_base_str, &to_component_str);

        ret = PVFS_sys_lookup(from_ref.fs_id, from_base_str, &creds,
                              &from_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                              NULL);
        if(from_base_str)
        {
            free(from_base_str);
        }
        if(ret < 0)
        {
            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_lookup(to_ref.fs_id, to_base_str, &creds,
                              &to_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                              NULL);
        if(to_base_str)
        {
            free(to_base_str);
        }

        if(ret < 0)
        {
            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_rename((char *)from_component_str, from_lookup_resp.ref,
                              (char *)to_component_str, to_lookup_resp.ref,
                              &creds, NULL);

        free(to_component_str);
        free(from_component_str);
    }

    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}

static int zint_pvfs2_link(const zoidfs_handle_t * UNUSED(from_parent_handle),
                    const char * UNUSED(from_component_name),
                    const char * UNUSED(from_full_path),
                    const zoidfs_handle_t * UNUSED(to_parent_handle),
                    const char * UNUSED(to_component_name),
                    const char * UNUSED(to_full_path),
                    zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                    zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    /* PVFS2 does not support hardlinks.  */
    return ZFSERR_NOTIMPL;
}

static int zint_pvfs2_symlink(const zoidfs_handle_t * from_parent_handle,
                       const char * from_component_name,
                       const char * from_full_path,
                       const zoidfs_handle_t * UNUSED(to_parent_handle),
                       const char * to_component_name,
                       const char * to_full_path,
                       const zoidfs_sattr_t * attr,
                       zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                       zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_sys_attr pattr;
    PVFS_credentials creds;
    PVFS_object_ref from_ref;
    PVFS_sysresp_symlink resp;
    PVFS_sysresp_lookup from_lookup_resp;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_sattr(attr, &pattr);

    zint_pvfs2_pattr_fill_setable(&pattr, 0755);

    zint_pvfs2_input_handle(from_parent_handle, from_ref);

    /* @TODO: FIX THIS (see ticket )*/
    if (from_component_name && to_component_name)
    {
        return ZFSERR_NOTIMPL;
    }
    else if (from_component_name && !to_component_name)
    {
        ret = PVFS_sys_symlink((char *)from_component_name, from_ref,
                               (char *)to_full_path, pattr, &creds, &resp,
                               NULL);
    }
    else if (!from_component_name && to_component_name)
    {
        return ZFSERR_NOTIMPL;
    }
    else
    {
        char * from_base_str;
        char * from_component_str;

        zint_pvfs2_split_path(from_full_path, &from_base_str,
                              &from_component_str);

        ret = PVFS_sys_lookup(from_ref.fs_id, from_base_str, &creds,
                              &from_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                              NULL);
        if(from_base_str)
        {
            free(from_base_str);
        }
        if(ret < 0)
        {
            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_symlink((char *)from_component_str,
                               from_lookup_resp.ref, (char *)to_full_path,
                               pattr, &creds, &resp, NULL);
        free(from_component_str);
    }

    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}

static int zint_pvfs2_mkdir(const zoidfs_handle_t * parent_handle,
                     const char * component_name,
                     const char * full_path,
                     const zoidfs_sattr_t * attr,
                     zoidfs_cache_hint_t * UNUSED(parent_hint),
                     zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_object_ref ref;
    PVFS_credentials creds;
    PVFS_sys_attr pattr;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_sysresp_mkdir mkdir_resp;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_sattr(attr, &pattr);

    zint_pvfs2_pattr_fill_setable(&pattr, 0755);

    zint_pvfs2_input_handle(parent_handle, ref);
    if(component_name)
    {
        ret = PVFS_sys_mkdir((char *)component_name, ref, pattr, &creds,
                             &mkdir_resp, NULL);
    }
    else
    {
        char * component_str;
        char * base_str;

        zint_pvfs2_split_path(full_path, &base_str, &component_str);

        ret = PVFS_sys_lookup(ref.fs_id, base_str, &creds, &lookup_resp, 
                              PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);

        if(base_str)
        {
            free(base_str);
        }

        if(ret < 0)
        {
            return zint_pvfs2_output_error(ret);
        }

        ret = PVFS_sys_mkdir(component_str, lookup_resp.ref, pattr, &creds,
                             &mkdir_resp, NULL);

        free(component_str);
    }

    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}

static int zint_pvfs2_readdir(const zoidfs_handle_t * parent_handle,
                       zoidfs_dirent_cookie_t cookie,
                       size_t * entry_count,
                       zoidfs_dirent_t * entries,
                       uint32_t flags,
                       zoidfs_cache_hint_t * UNUSED(parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    size_t i;
    PVFS_object_ref ref;
    PVFS_credentials creds;
    PVFS_sysresp_readdir readdir_resp;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_handle(parent_handle, ref);

    ret = PVFS_sys_readdir(ref, (PVFS_ds_position)cookie, *entry_count, &creds,
                           &readdir_resp, NULL);
    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    *entry_count = readdir_resp.pvfs_dirent_outcount;
    for(i = 0; i < *entry_count; i++)
    {
        PVFS_object_ref entry_ref = ref;
        strncpy(entries[i].name, readdir_resp.dirent_array[i].d_name,
                ZOIDFS_NAME_MAX);
        entries[i].name[ZOIDFS_NAME_MAX] = 0;

        entry_ref.handle = readdir_resp.dirent_array[i].handle;

        if (flags & ZOIDFS_RETR_HANDLE)
        {
            zint_pvfs2_output_handle(entry_ref, &entries[i].handle);
        }

        if (flags & ZOIDFS_ATTR_ALL)
        {
            uint32_t mask;
            PVFS_sysresp_getattr getattr_resp;

            mask = zint_pvfs2_input_attr_mask(flags & ZOIDFS_ATTR_ALL);

            ret = PVFS_sys_getattr(entry_ref, mask, &creds, &getattr_resp,
                                   NULL);
            if(ret < 0)
            {
                free(readdir_resp.dirent_array);
                return zint_pvfs2_output_error(ret);
            }

            zint_pvfs2_output_attr(getattr_resp.attr, &entries[i].attr);
        }

        entries[i].cookie = (zoidfs_dirent_cookie_t)readdir_resp.token;
    }

    if (*entry_count)
       free(readdir_resp.dirent_array);

    return ZFS_OK;
}

static int zint_pvfs2_resize(const zoidfs_handle_t * handle, uint64_t size,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int ret;
    PVFS_object_ref ref;
    PVFS_credentials creds;

    PVFS_util_gen_credentials(&creds);

    zint_pvfs2_input_handle(handle, ref);

    ret = PVFS_sys_truncate(ref, size, &creds, NULL);
    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}

static int zint_pvfs2_resolve_path(const char * local_path,
                            char * fs_path,
                            int fs_path_max,
                            zoidfs_handle_t * newhandle,
                            int * usenew)
{
    int ret;
    PVFS_object_ref ref;
    PVFS_fs_id fsid;

    ret = PVFS_util_resolve(local_path, &fsid, fs_path, fs_path_max);
    
     ref.fs_id = fsid;
     ref.handle = 0;
 
     if(newhandle)
     {
         zint_pvfs2_output_handle(ref, newhandle);
         *usenew = 1; 
     }


    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}


/**
 * OPTIONAL
 */
static int zint_pvfs2_init(void)
{
    int ret;
    ret = PVFS_util_init_defaults();

    if(ret < 0)
    {
        return zint_pvfs2_output_error(ret);
    }

    return ZFS_OK;
}

/**
 * OPTIONAL
 */
static int zint_pvfs2_finalize(void)
{
    PVFS_sys_finalize();
    return ZFS_OK;
}

zint_handler_t pvfs2_handler =
{
    zint_pvfs2_null,
    zint_pvfs2_getattr,
    zint_pvfs2_setattr,
    zint_pvfs2_lookup,
    zint_pvfs2_readlink,
    zint_pvfs2_read,
    zint_pvfs2_write,
    zint_pvfs2_commit,
    zint_pvfs2_create,
    zint_pvfs2_remove,
    zint_pvfs2_rename,
    zint_pvfs2_link,
    zint_pvfs2_symlink,
    zint_pvfs2_mkdir,
    zint_pvfs2_readdir,
    zint_pvfs2_resize,
    zint_pvfs2_resolve_path,
    zint_pvfs2_init,
    zint_pvfs2_finalize
};

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ts=8 sts=4 sw=4 expandtab
 */
