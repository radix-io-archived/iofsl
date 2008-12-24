/****************************************************************************/
/* ZEPTOOS:zepto-info */
/*     This file is part of ZeptoOS: The Small Linux for Big Computers.
 *     See www.mcs.anl.gov/zeptoos for more information.
 */
/* ZEPTOOS:zepto-info */
/* */
/* ZEPTOOS:zepto-fillin */
/*     $Id: zoidfs.h,v 1.10 2007/06/27 15:10:45 iskra Exp $
 *     ZeptoOS_Version: 1.2
 *     ZeptoOS_Heredity: FOSS_ORIG
 *     ZeptoOS_License: GPL
 */
/* ZEPTOOS:zepto-fillin */
/* */
/* ZEPTOOS:zepto-gpl */
/*      Copyright: Argonne National Laboratory, Department of Energy,
 *                 and UChicago Argonne, LLC.  2004, 2005, 2006, 2007
 *      ZeptoOS License: GPL
 * 
 *      This software is free.  See the file ZeptoOS/misc/license.GPL
 *      for complete details on your rights to copy, modify, and use this
 *      software.
 */
/* ZEPTOOS:zepto-gpl */
/****************************************************************************/

#ifndef ZOIDFS_H
#define ZOIDFS_H

#define ZOIDFS_NULL_COOKIE 0
#define ZOIDFS_NAME_MAX 256
#define ZOIDFS_PATH_MAX 4096
#define ZOIDFS_BUFFER_MAX (1 * 1024 * 1024)

#include <stdint.h>
#include <stddef.h>

/* Op id describes the various zoidfs operations (setattr, getattr etc.) */
typedef unsigned int zoidfs_op_id_t;

/*
 * null_param is used by the server to determine whether the client is passing
 * either the full_path or the parent_handle and component_name.
 */
typedef int zoidfs_null_param_t;

/*
 * zoidfs_op_status_t is used by the server to inform the client of the status
 * of the operation.
 */
typedef int zoidfs_op_status_t;

/**
 * Handle is an opaque 16 byte value
 */
typedef struct
{
    uint8_t data[16];
} zoidfs_handle_t;

/* The cookie is an opaque object of 4 bytes */
typedef uint32_t zoidfs_dirent_cookie_t;

typedef struct
{
    char name[ZOIDFS_NAME_MAX+1];
    zoidfs_handle_t handle;
    zoidfs_dirent_cookie_t cookie;
} zoidfs_dirent_t;

typedef enum 
{
    ZOIDFS_NON   = 0,
    ZOIDFS_REG   = 1,
    ZOIDFS_DIR   = 2,
    ZOIDFS_INVAL = 3
} zoidfs_attr_type_t;

#define ZOIDFS_ATTR_MODE   (1 << 0)
#define ZOIDFS_ATTR_NLINK  (1 << 1)
#define ZOIDFS_ATTR_UID    (1 << 2)
#define ZOIDFS_ATTR_GID    (1 << 3)
#define ZOIDFS_ATTR_SIZE   (1 << 4)
#define ZOIDFS_ATTR_BSIZE  (1 << 5)
#define ZOIDFS_ATTR_FSID   (1 << 6)
#define ZOIDFS_ATTR_FILEID (1 << 7)
#define ZOIDFS_ATTR_ATIME  (1 << 8)
#define ZOIDFS_ATTR_MTIME  (1 << 9)
#define ZOIDFS_ATTR_CTIME  (1 << 10)

#define ZOIDFS_ATTR_SETABLE (ZOIDFS_ATTR_MODE  | \
                             ZOIDFS_ATTR_UID   | \
                             ZOIDFS_ATTR_GID   | \
                             ZOIDFS_ATTR_ATIME | \
                             ZOIDFS_ATTR_MTIME)

#define ZOIDFS_ATTR_ALL (ZOIDFS_ATTR_MODE  | \
                         ZOIDFS_ATTR_NLINK | \
                         ZOIDFS_ATTR_UID   | \
                         ZOIDFS_ATTR_GID   | \
                         ZOIDFS_ATTR_SIZE  | \
                         ZOIDFS_ATTR_BSIZE | \
                         ZOIDFS_ATTR_FSID  | \
                         ZOIDFS_ATTR_FILEID| \
                         ZOIDFS_ATTR_ATIME | \
                         ZOIDFS_ATTR_MTIME | \
                         ZOIDFS_ATTR_CTIME)

typedef struct
{
    uint64_t seconds;
    uint32_t useconds;
} zoidfs_time_t;

typedef struct
{
    zoidfs_attr_type_t type;
    uint16_t mask;
    uint16_t mode;
    uint32_t nlink;
    uint32_t uid;
    uint32_t gid;
    uint64_t size;
    uint32_t blocksize;
    uint32_t fsid;
    uint64_t fileid;
    zoidfs_time_t atime;
    zoidfs_time_t mtime;
    zoidfs_time_t ctime;
} zoidfs_attr_t;

typedef struct
{
    uint64_t size;
    zoidfs_time_t atime;
    zoidfs_time_t mtime;
    zoidfs_time_t ctime;
} zoidfs_cache_hint_t;

typedef struct
{
    uint16_t mask;
    uint16_t mode;
    uint32_t uid;
    uint32_t gid;
    uint64_t size;
    zoidfs_time_t atime;
    zoidfs_time_t mtime;
} zoidfs_sattr_t;

/**
 * XXX: These need to be sorted by severity
 */
enum {
    ZFS_OK = 0,
    ZFSERR_PERM=1,
    ZFSERR_NOENT=2,
    ZFSERR_IO=5,
    ZFSERR_NXIO=6,
    ZFSERR_ACCES=13,
    ZFSERR_EXIST=17,
    ZFSERR_NODEV=19,
    ZFSERR_NOTDIR=20,
    ZFSERR_ISDIR=21,
    ZFSERR_FBIG=27,
    ZFSERR_NOSPC=28,
    ZFSERR_ROFS=30,
    ZFSERR_NAMETOOLONG=63,
    ZFSERR_NOTEMPTY=66,
    ZFSERR_DQUOT=69,
    ZFSERR_STALE=70,
    ZFSERR_WFLUSH=99,
    ZFSERR_XDR=100,
    ZFSERR_MISC=101
};

/* START-ZOID-SCANNER ID=1 INIT=NULL FINI=NULL PROC=NULL */

void zoidfs_null(void);

int zoidfs_getattr(const zoidfs_handle_t * handle /* in:ptr */,
                   zoidfs_attr_t * attr /* out:ptr */);

int zoidfs_setattr(const zoidfs_handle_t * handle /* in:ptr */,
                   const zoidfs_sattr_t * sattr /* in:ptr */,
                   zoidfs_attr_t * attr /* out:ptr:nullok */);

int zoidfs_lookup(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  zoidfs_handle_t * handle /* out:ptr */);

int zoidfs_readlink(const zoidfs_handle_t * handle /* in:ptr */,
                    char * buffer /* out:arr:size=+1 */,
                    size_t buffer_length /* in:obj */);

int zoidfs_read(const zoidfs_handle_t * handle /* in:ptr */,
                int mem_count /* in:obj */,
                void * mem_starts[] /* out:arr2d:size=+1:zerocopy */,
                const size_t mem_sizes[] /* in:arr:size=-2 */,
                int file_count /* in:obj */,
                const uint64_t file_starts[] /* in:arr:size=-1 */,
                uint64_t file_sizes[] /* inout:arr:size=-2 */);

int zoidfs_write(const zoidfs_handle_t * handle /* in:ptr */,
                 int mem_count /* in:obj */,
                 const void * mem_starts[] /* in:arr2d:size=+1:zerocopy */,
                 const size_t mem_sizes[] /* in:arr:size=-2 */,
                 int file_count /* in:obj */,
                 const uint64_t file_starts[] /* in:arr:size=-1 */,
                 uint64_t file_sizes[] /* inout:arr:size=-2 */);

int zoidfs_commit(const zoidfs_handle_t * handle /* in:ptr */);

int zoidfs_create(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  const zoidfs_sattr_t * attr /* in:ptr */,
                  zoidfs_handle_t * handle /* out:ptr */,
                  int * created /* out:ptr:nullok */);

int zoidfs_remove(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  zoidfs_cache_hint_t * parent_hint /* out:ptr:nullok */);

int zoidfs_rename(const zoidfs_handle_t * from_parent_handle /* in:ptr:nullok */,
                  const char * from_component_name /* in:str:nullok */,
                  const char * from_full_path /* in:str:nullok */,
                  const zoidfs_handle_t * to_parent_handle /* in:ptr:nullok */,
                  const char * to_component_name /* in:str:nullok */,
                  const char * to_full_path /* in:str:nullok */,
                  zoidfs_cache_hint_t * from_parent_hint /* out:ptr:nullok */,
                  zoidfs_cache_hint_t * to_parent_hint /* out:ptr:nullok */);

int zoidfs_symlink(const zoidfs_handle_t * from_parent_handle /* in:ptr:nullok */,
                   const char * from_component_name /* in:str:nullok */,
                   const char * from_full_path /* in:str:nullok */,
                   const zoidfs_handle_t * to_parent_handle /* in:ptr:nullok */,
                   const char * to_component_name /* in:str:nullok */,
                   const char * to_full_path /* in:str:nullok */,
                   const zoidfs_sattr_t * attr /* in:ptr */,
                   zoidfs_cache_hint_t * from_parent_hint /* out:ptr:nullok */,
                   zoidfs_cache_hint_t * to_parent_hint /* out:ptr:nullok */);

int zoidfs_mkdir(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                 const char * component_name /* in:str:nullok */,
                 const char * full_path /* in:str:nullok */,
                 const zoidfs_sattr_t * attr /* in:ptr */,
                 zoidfs_cache_hint_t * parent_hint /* out:ptr:nullok */);

int zoidfs_readdir(const zoidfs_handle_t * parent_handle /* in:ptr */,
                   zoidfs_dirent_cookie_t cookie /* in:obj */,
                   int * entry_count /* inout:ptr */,
                   zoidfs_dirent_t * entries /* out:arr:size=-1*/,
                   zoidfs_cache_hint_t * parent_hint /* out:ptr:nullok */);

int zoidfs_resize(const zoidfs_handle_t * handle /* in:ptr */,
                  uint64_t size /* in:obj */);


/**
 * OPTIONAL
 */
int zoidfs_init(void);

/**
 * OPTIONAL
 */
int zoidfs_finalize(void);

#endif /* ZOIDFS_H */

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
