/*
 * zoidfs-sysio.c
 * SYSIO driver for the ZOIDFS API.
 *
 * Jason Cope <copej@mcs.anl.gov>
 * Nawab Ali <alin@cse.ohio-state.edu>
 * 
 */

#include "zoidfs-sysio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include <getopt.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>

#if defined(SYSIO_LABEL_NAMES)
#include "sysio.h"
#endif

#include "xtio.h"
#include "fhi.h"

#include "zoidfs.h"
#include "c-util/tools.h"

/*
 * zoidfs sysio init variables
 * ... make sure we only init once
 */
static int sysio_dispatcher_initialized = 0;
static int sysio_dispatcher_ref_count = 0;
static pthread_mutex_t sysio_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * root zoidfs handle
 */
static char zoidfs_sysio_root_path[4096];
static char zoidfs_sysio_root_handle_data[SYSIO_HANDLE_DATA_SIZE];
static struct file_handle_info zoidfs_sysio_root_handle = {NULL, zoidfs_sysio_root_handle_data, SYSIO_HANDLE_DATA_SIZE};

/*
 * user specified zoidfs handle and path
 */
static char zoidfs_sysio_mfs_path[4096];
static char zoidfs_sysio_mfs_handle_data[SYSIO_HANDLE_DATA_SIZE];
static struct file_handle_info zoidfs_sysio_mfs_handle = {NULL, zoidfs_sysio_mfs_handle_data, SYSIO_HANDLE_DATA_SIZE};

/*
 * zoidfs sysio trace and debug tools
 */

 
#define ZOIDFS_SYSIO_DEBUG
#define ZFSSYSIO_TRACE_ENABLED


/*
 * print enter trace statement
 */
#ifdef ZFSSYSIO_TRACE_ENABLED
#define ZFSSYSIO_TRACE_ENTER \
	do { \
        fprintf(stderr, "%s %s, ZOIDFS SYSIO DISPATCHER - ENTER %s() %s:%i\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__); \
	}while(0)
#else
#define ZFSSYSIO_TRACE_ENTER \
	/* trace disabled */
#endif /* ZFSSYSIO_TRACE_ENABLED */

/*
 * print exit trace statement
 */
#ifdef ZFSSYSIO_TRACE_ENABLED
#define ZFSSYSIO_TRACE_EXIT \
	do { \
        fprintf(stderr, "%s %s, ZOIDFS SYSIO DISPATCHER - EXIT %s() %s:%i\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__); \
	}while(0)
#else
#define ZFSSYSIO_TRACE_EXIT
	/* trace disabled */
#endif /* ZFSSYSIO_TRACE_ENABLED */ 

#ifdef ZOIDFS_SYSIO_DEBUG
#define ZFSSYSIO_INFO(__format, ...) \
    do { \
        char buffer[4096]; \
        sprintf(buffer, __format, __VA_ARGS__); \
        fprintf(stderr, "%s %s, ZOIDFS SYSIO DISPATCHER - INFO %s() %s:%i : %s\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__, buffer); \
    }while(0)
#endif /* ZOIDFS_SYSIO_DEBUG */

/* 
 * determine the static size of print storage buffers for the handles
*/
#ifndef  _ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE
#define  _ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE (2 * ZOIDFS_HANDLE_PAYLOAD_SIZE + 1)
#endif /*  _ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE */

/*
 * Dump the hex values for a zoidfs handle to stderr
 */
static inline int zoidfs_print_handle(const zoidfs_handle_t * handle)
{
    uint8_t v[ZOIDFS_HANDLE_PAYLOAD_SIZE];
    unsigned char vstr[_ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE];
    unsigned char * buffer = NULL;
    unsigned char * vptr = NULL;
    int i = 0;

    memcpy(v, handle->data + ZOIDFS_HANDLE_HEADER_SIZE, ZOIDFS_HANDLE_PAYLOAD_SIZE);
    vstr[_ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE - 1] = 0;
    
    buffer = (unsigned char *)v;
    vptr = vstr;
    i = 0;
    while(i < ZOIDFS_HANDLE_PAYLOAD_SIZE)
    {
        int j = 0;
        unsigned char cb = buffer[i];
        while(j < 2)
        {
            unsigned int val = (cb >> 4) & 0xf;
            if(val < 10)
            {
                *vptr++ = (unsigned char)('0' + val);
            }
            else
            {
                *vptr++ = (unsigned char)('a' + (val - 10));
            }
            cb = (cb << 4);
            j += 1;
        }
        i += 1;
    }
    fprintf(stderr, "zfs handle data:   %s\n", vstr);
    return ZFS_OK;
}

/*
 * Dump the hex values for a sysio handle stderr
 */
static inline int sysio_print_handle(const struct file_handle_info * handle)
{
    uint8_t v[ZOIDFS_HANDLE_PAYLOAD_SIZE];
    unsigned char vstr[_ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE];
    unsigned char * buffer = NULL;
    unsigned char * vptr = NULL;
    int i = 0;
    
    vstr[_ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE - 1] = 0;

    memcpy(v, &handle->fhi_export, SYSIO_FHE_SIZE);
    memcpy(v + SYSIO_FHE_SIZE, handle->fhi_handle, SYSIO_HANDLE_DATA_SIZE);
    v[SYSIO_FHE_SIZE + SYSIO_HANDLE_DATA_SIZE] = (uint8_t)handle->fhi_handle_len;

    buffer = (unsigned char *)v;
    vptr = vstr;
    i = 0;
    while(i < ZOIDFS_HANDLE_PAYLOAD_SIZE)
    {
        int j = 0;
        unsigned char cb = buffer[i];
        while(j < 2)
        {
            unsigned int val = (cb >> 4) & 0xf;
            if(val < 10)
            {
                *vptr++ = (unsigned char)('0' + val);
            }
            else
            {
                *vptr++ = (unsigned char)('a' + (val - 10));
            }
            cb = (cb << 4);
            j += 1;
        }
        i += 1;
    }
    fprintf(stderr, "sysio handle data: %s\n", vstr);
    return ZFS_OK;
}

static inline int zoidfs_sysio_handle_cmp(const struct file_handle_info * sysio_handle, const zoidfs_handle_t * zoidfs_handle)
{
    uint8_t v[ZOIDFS_HANDLE_PAYLOAD_SIZE];

    memcpy(v, &sysio_handle->fhi_export, SYSIO_FHE_SIZE);
    memcpy(v + SYSIO_FHE_SIZE, sysio_handle->fhi_handle, SYSIO_HANDLE_DATA_SIZE);
    v[SYSIO_FHE_SIZE + SYSIO_HANDLE_DATA_SIZE] = (uint8_t)sysio_handle->fhi_handle_len;

    if(memcmp(v, zoidfs_handle->data + ZOIDFS_HANDLE_HEADER_SIZE, ZOIDFS_HANDLE_PAYLOAD_SIZE) == 0)
    {
        fprintf(stderr, "zoidfs-sysio handle cmp, handles are equal\n");
    }
    else
    {
        fprintf(stderr, "zoidfs-sysio handle cmp, handles are not equal\n");
        if(memcmp(v, zoidfs_handle->data + ZOIDFS_HANDLE_HEADER_SIZE, SYSIO_FHE_SIZE) != 0)
        {
            fprintf(stderr, "zoidfs-sysio handle cmp, fhe_export differs\n");
        }
        if(memcmp(v + SYSIO_FHE_SIZE, zoidfs_handle->data + ZOIDFS_HANDLE_HEADER_SIZE + SYSIO_FHE_SIZE, SYSIO_HANDLE_DATA_SIZE) != 0)
        {
            fprintf(stderr, "zoidfs-sysio handle cmp, fhi_handle differs\n");
        }
        if(memcmp(v + SYSIO_FHE_SIZE + SYSIO_HANDLE_DATA_SIZE, zoidfs_handle->data + ZOIDFS_HANDLE_HEADER_SIZE + SYSIO_FHE_SIZE + SYSIO_HANDLE_DATA_SIZE, SYSIO_FHILENPACK_SIZE) != 0)
        {
            fprintf(stderr, "zoidfs-sysio handle cmp, fh_handle_len differs\n");
        }
    }

    return ZFS_OK;
}

/*
 * zoidfs and sysio handle conversion functions
 */

static inline int sysio_handle_init(struct file_handle_info * _sysio_h_ref)
{
    _sysio_h_ref->fhi_export = 0;
    _sysio_h_ref->fhi_handle_len = 0;
    memset(_sysio_h_ref->fhi_handle, 0, SYSIO_HANDLE_DATA_SIZE);
    return ZFS_OK;
}

static inline int zoidfs_handle_init(zoidfs_handle_t * _zoidfs_h_ref)
{
    memset(_zoidfs_h_ref, 0, ZOIDFS_HANDLE_DATA_SIZE);
    return ZFS_OK;
}

/*
 * Pack a sysio handle into a zodifs handle
 */
static inline int sysio_handle_to_zoidfs_handle(const struct file_handle_info * _sysio_h_ref, zoidfs_handle_t * _zoidfs_h_ref)
{
	uint8_t * _hptr = (uint8_t *)(_zoidfs_h_ref)->data;
	uint8_t * fhl;
	uint8_t * fhd;
	struct file_handle_info_export ** fhe;

    /*
     * zero out the zoidfs handle the sysio is being packed into
     */
    zoidfs_handle_init(_zoidfs_h_ref);

    _hptr += ZOIDFS_HANDLE_HEADER_SIZE;
	fhe = (struct file_handle_info_export **)_hptr;                 /* skip the zoidfs header */
	*fhe = (_sysio_h_ref)->fhi_export;                              /* copy the sysio export pointer */
    _hptr += SYSIO_FHE_SIZE;
	fhd = (uint8_t *)_hptr;                                         /* copy the sysio data payload */
	memcpy(fhd, (_sysio_h_ref)->fhi_handle, SYSIO_HANDLE_DATA_SIZE);
    _hptr += SYSIO_HANDLE_DATA_SIZE;
	fhl = (uint8_t *)_hptr;                                         /* copy the sysio data payload length */
	*fhl = (uint8_t)((_sysio_h_ref)->fhi_handle_len);

   
#ifdef ZOIDFS_SYSIO_DEBUG 
    fprintf(stderr, "s2z handle data\n");  
    zoidfs_sysio_handle_cmp(_sysio_h_ref, _zoidfs_h_ref);
    sysio_print_handle(_sysio_h_ref);
    zoidfs_print_handle(_zoidfs_h_ref);
#endif

	return ZFS_OK;
}

/*
 * Unpack a zoidfs handle into a sysio handle
 */ 
static inline int zoidfs_handle_to_sysio_handle(const zoidfs_handle_t * _zoidfs_h_ref, struct file_handle_info * _sysio_h_ref)
{
	uint8_t * _hptr = (uint8_t *)(_zoidfs_h_ref)->data;
	uint8_t * fhl;
	uint8_t * fhd;
	struct file_handle_info_export ** fhe;

    /*
     * zero out the the sysio handle the zoidfs handle is being unpacked into
     */
    sysio_handle_init(_sysio_h_ref);

    /*
     * unpack the zoidfs handle
     */
    _hptr += ZOIDFS_HANDLE_HEADER_SIZE;                             /* skip the zoidfs header */
	fhe = (struct file_handle_info_export **)_hptr;                 /* setup the sysio export pointer */
	(_sysio_h_ref)->fhi_export = *fhe;
    _hptr += SYSIO_FHE_SIZE;                                        
	fhd = (uint8_t *)_hptr;                                         /* setup the sysio data payload field */
	memcpy((_sysio_h_ref)->fhi_handle, fhd, SYSIO_HANDLE_DATA_SIZE);
    _hptr += SYSIO_HANDLE_DATA_SIZE;
	fhl = (uint8_t *)_hptr;                                         /* setup the sysio data payload length field */
	(_sysio_h_ref)->fhi_handle_len = (size_t)(*fhl);

#ifdef ZOIDFS_SYSIO_DEBUG 
    fprintf(stderr, "z2s handle data\n");  
    zoidfs_sysio_handle_cmp(_sysio_h_ref, _zoidfs_h_ref);
    sysio_print_handle(_sysio_h_ref);
    zoidfs_print_handle(_zoidfs_h_ref); 
#endif 

	return ZFS_OK;
}

/*
 * zfs sysio error handler
 * sysio errs use posix errno codes... reuse the zfs posix dsipatcher
 * error handler code here...
 */
static inline int sysio_err_to_zfs_err(int val)
{
   if (val == 0)
      return ZFS_OK; 
   switch(val)
   {
      case EPERM:
         return ZFSERR_PERM;
      case ENOENT:
         return ZFSERR_NOENT;
      case EIO: 
         return ZFSERR_IO;
      case ENXIO: 
         return ZFSERR_NXIO;
      case EACCES:
         return ZFSERR_ACCES;
      case EEXIST:
         return ZFSERR_EXIST;
      case ENODEV:
         return ZFSERR_NODEV;
      case ENOTDIR:
         return ZFSERR_NOTDIR;
      case EISDIR: 
         return ZFSERR_ISDIR;
      case EFBIG:
         return ZFSERR_FBIG;
      case ENOSPC:
         return ZFSERR_NOSPC;
      case EROFS:
         return ZFSERR_ROFS;
      case ENAMETOOLONG:
         return ZFSERR_NAMETOOLONG;
      case ENOTEMPTY:
         return ZFSERR_NOTEMPTY;
      case EDQUOT:
         return ZFSERR_DQUOT;
      case ESTALE:
         return ZFSERR_STALE;
      default:
         return ZFSERR_OTHER; 
   }
}

/*
 * zoidfs_sysio_export
 * This function does a nop
 */
static int zoidfs_sysio_null(void) {
	ZFSSYSIO_TRACE_ENTER;
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}

/*
 * zoidfs_sysio_export
 * This function exports the base path of a remote file system and generates a
 * libsysio compatiable handle
 */
int zoidfs_sysio_export(int * key, char * base_path, struct file_handle_info * root_handle)
{
    int ret = 0;

	ZFSSYSIO_TRACE_ENTER;
    
    /*
     * export remote_fs_path based on key
     */
    ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_export)(key, sizeof(*key), base_path, 0, &(root_handle->fhi_export));
    if (ret) {
	fprintf(stderr, "zoidfs_sysio_export: fhi_export() failed.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}

/*
 * zoidfs_sysio_export
 * This function exports the base path of a remote file system and generates a
 * libsysio compatiable handle
 */
int zoidfs_sysio_unexport(struct file_handle_info * root_handle)
{
    int ret = 0;

	ZFSSYSIO_TRACE_ENTER;
    
    /*
     * export remote_fs_path based on key
     */
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_unexport)(root_handle->fhi_export);
    if (ret) {
		fprintf(stderr, "zoidfs_sysio_unexport: fhi_unexport() failed.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}

/*
 * zoidfs_sysio_rootof
 * This function identifies the file system root using the fhi_root_of libsysio function
 *
 */
int zoidfs_sysio_rootof(struct file_handle_info * root_handle)
{
    int ret = 0;

	ZFSSYSIO_TRACE_ENTER;
	
    /*
     * find the root from the handle
     */
    ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_root_of)(root_handle->fhi_export, root_handle);
    if (ret < 0) {
	fprintf(stderr, "zoidfs_libsysio_rootof: fhi_root_of() failed.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
    /*
     * Verify that the handle data is not too large
     */
    if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
	fprintf(stderr, "zoidfs_sysio_rootof: handle data too large, %lu >= %lu\n", (unsigned long)ret, (unsigned long)root_handle->fhi_handle_len);
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
	
    ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}

/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle.
 */
static int zoidfs_sysio_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr)
{
    int ret;
    struct stat64 stbuf;
    char sysio_handle_data[SYSIO_HANDLE_DATA_SIZE];
    struct file_handle_info sysio_handle = {NULL, (void *)sysio_handle_data, (size_t)SYSIO_HANDLE_DATA_SIZE};

	ZFSSYSIO_TRACE_ENTER;

	/*
	 * Convert the zoidfs handle to a sysio handle
	 * and initialize the handle
	 */ 
	zoidfs_handle_to_sysio_handle(handle, &sysio_handle);
	
	/*
	 * execute the sysio getattr call
	 */
    ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getattr)(&sysio_handle, &stbuf);
    if (ret) {
        fprintf(stderr, "zoidfs_sysio_getattr: fhi_getattr() failed, code = %i.\n", ret);
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

    /* Convert the SYSIO attributes to ZOIDFS attributes */
	if (S_ISDIR(stbuf.st_mode))
        attr->type = ZOIDFS_DIR;
	else if (S_ISCHR(stbuf.st_mode))
        attr->type = ZOIDFS_CHR;
	else if (S_ISBLK(stbuf.st_mode))
        attr->type = ZOIDFS_BLK;
	else if (S_ISREG(stbuf.st_mode))
        attr->type = ZOIDFS_REG;
#ifdef S_ISFIFO
	else if (S_ISFIFO(stbuf.st_mode))
        attr->type = ZOIDFS_FIFO;
#endif
#ifdef S_ISLNK
	else if (S_ISLNK(stbuf.st_mode))
        attr->type = ZOIDFS_LNK;
#endif
#ifdef S_ISSOCK
	else if (S_ISSOCK(stbuf.st_mode))
        attr->type = ZOIDFS_SOCK;
#endif
	else
        attr->type = ZOIDFS_INVAL;

	/* always set all attrs */
    attr->mask =	ZOIDFS_ATTR_MODE |
					ZOIDFS_ATTR_NLINK | 
					ZOIDFS_ATTR_UID |
					ZOIDFS_ATTR_GID |
					ZOIDFS_ATTR_SIZE |
					ZOIDFS_ATTR_ATIME |
					ZOIDFS_ATTR_CTIME |
					ZOIDFS_ATTR_MTIME;
	
    attr->mode = stbuf.st_mode & 0777;
    attr->nlink = stbuf.st_nlink;
    attr->uid = stbuf.st_uid;
    attr->gid = stbuf.st_gid;
    attr->size = stbuf.st_size;
    attr->blocksize = stbuf.st_blksize;
    attr->fsid = 0;     /* can't get this value... set to 0 */
    attr->fileid = 0;   /* can't get this value... set to 0 */
    attr->atime.seconds = stbuf.st_atime;
    attr->atime.nseconds = 0;
    attr->mtime.seconds = stbuf.st_mtime;
    attr->mtime.nseconds = 0;
    attr->ctime.seconds = stbuf.st_ctime;
    attr->ctime.nseconds = 0;

	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
static int zoidfs_sysio_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t *attr) {
	
    int ret;
    struct file_handle_info_sattr sysio_sattr;

	int setAttrs = 0;
    char sysio_handle_data[SYSIO_HANDLE_DATA_SIZE];
    struct file_handle_info sysio_handle = {NULL, sysio_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;

	/*
	 * Convert the zoidfs handle to a sysio handle
	 * and initialize the handle
	 */ 
	zoidfs_handle_to_sysio_handle(handle, &sysio_handle);
	
	/*
	 * Set the mode
	 */
	if(sattr->mask & ZOIDFS_ATTR_MODE)
	{
		sysio_sattr.fhisattr_mode = sattr->mode;
		sysio_sattr.fhisattr_mode_set = 1;
		setAttrs = 1;
	}
	/*
	 * Set the UID
	 */
	if(sattr->mask & ZOIDFS_ATTR_UID)
	{
		sysio_sattr.fhisattr_uid = sattr->uid;
		sysio_sattr.fhisattr_uid_set = 1;
		setAttrs = 1;
	}
	/*
	 * Set the GID
	 */
	if(sattr->mask & ZOIDFS_ATTR_GID)
	{
		sysio_sattr.fhisattr_gid = sattr->gid;
		sysio_sattr.fhisattr_gid_set = 1;
		setAttrs = 1;
	}
	/*
	 * Set the MTIME
	 */
	if(sattr->mask & ZOIDFS_ATTR_MTIME)
	{
		sysio_sattr.fhisattr_mtime = sattr->mtime.seconds;
		sysio_sattr.fhisattr_mtime_set = 1;
		setAttrs = 1;
	}
	/*
	 * Set the ATIME
	 */
	if(sattr->mask & ZOIDFS_ATTR_ATIME)
	{
		sysio_sattr.fhisattr_atime = sattr->atime.seconds;
		sysio_sattr.fhisattr_atime_set = 1;
		setAttrs = 1;
	}
	/*
	 * Set the SIZE
	 */
	if(sattr->mask & ZOIDFS_ATTR_SIZE)
	{
		sysio_sattr.fhisattr_size = sattr->size;
		sysio_sattr.fhisattr_size_set = 1;
		setAttrs = 1;
	}
	
	/*
	 * Execute the sysio setattr call
	 */
	if(setAttrs)
	{
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_setattr)(&sysio_handle, &sysio_sattr);
		if (ret) {
			fprintf(stderr, "zoidfs_sysio_setattr: fhi_setattr() failed.\n");
			perror("zoidfs_sysio_setattr");
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	
	/*
	 * Execute the zoidfs_sysio getattr call and populate the attr struct
	 */
	ret = zoidfs_sysio_getattr(handle, attr);
	if (ret != ZFS_OK) {
		fprintf(stderr, "zoidfs_sysio_setattr: fhi_getattr() failed.\n");
		perror("zoidfs_sysio_setattr");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 */
static int zoidfs_sysio_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length) {
	
    int ret;
	struct stat64 stbuf;
	char sysio_link_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_link_handle = {NULL, sysio_link_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;
	
	zoidfs_handle_to_sysio_handle(handle, &sysio_link_handle);
	/*
	 * Is this a link?
	 */
	
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getattr)(&sysio_link_handle, &stbuf);
	if (ret) {
		perror("fhi_getattr");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}

	if (S_ISLNK(stbuf.st_mode)) {
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_readlink)(&sysio_link_handle, buffer, buffer_length);
		if (ret < 0) {
			perror("readlink");
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_lookup
 * This function returns the file handle associated with the given file or
 * directory name.
 */
static int zoidfs_sysio_lookup(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_handle_t *handle) {

	ZFSSYSIO_TRACE_ENTER;
	
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_sysio_lookup: Invalid path parameters.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Full path given
	 */
    if (full_path) {
		int ret = 0;
		/*
		 * sysio parent handle
		 */
		char h_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info h = {NULL, h_data, SYSIO_HANDLE_DATA_SIZE};
	
		/*
		 * Lookup the exported file info for the parent handle
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path, 0, &h);
		if(ret < 0)
		{
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
		if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
			fprintf(stderr, "zoidfs_sysio_rootof: handle data too large, %lu >= %lu\n", (unsigned long)ret, (unsigned long)zoidfs_sysio_root_handle.fhi_handle_len);
			(void )fprintf(stderr, "handle data too large\n");
		}
		sysio_handle_to_zoidfs_handle(&h, handle);

	/*
	 * Else, parent handle and component_name given
	 */
    } else {
		/*
		 * sysio parent handle
		 */
		char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
		
		/*
		 * sysio component handle
		 */
		char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
		int ret = 0;
		
		/*
		 * Convert the zoidfs parent handle to the sysio handle
		 */
		zoidfs_handle_to_sysio_handle(parent_handle, &sysio_parent_handle);
		
		/*
		 * Lookup the exported file info for the parent handle
		 */

		/* Make sure the file does not already exist*/
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&sysio_parent_handle, component_name, 0, &sysio_component_handle);
		if(ret < 0)
		{
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
		if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
			fprintf(stderr, "zoidfs_sysio_rootof: handle data too large, %i >= %i\n", ret, SYSIO_HANDLE_DATA_SIZE);
			(void )fprintf(stderr, "handle data too large\n");
		}
		sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
    }
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_remove
 * This function removes the given file or directory.
 */
static int zoidfs_sysio_remove(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_cache_hint_t * UNUSED(parent_hint)) {
    int ret;
    char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
    struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
    struct file_handle_info_dirop_args where;
	char sysio_fp_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_fp_handle = {NULL, sysio_fp_handle_data, SYSIO_HANDLE_DATA_SIZE};
	struct stat64 stbuf;

	ZFSSYSIO_TRACE_ENTER;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_sysio_remove: Invalid path parameters.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
	
	if(full_path)
	{
		int ret = 0;
		
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path, 0, &sysio_fp_handle);
		if (ret < 0) {
			perror("fhi_lookup");
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
		if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
			(void )fprintf(stderr,
					   "%s: handle data too large\n",
					   component_name);
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	else
	{
		/*
		 * Setup the file handle based on the libsysio handle and the file handle
		 */
		zoidfs_handle_to_sysio_handle(parent_handle, &sysio_parent_handle);
	
		/*
		 * Is this a file or a directory?
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&sysio_parent_handle, component_name, 0, &sysio_fp_handle);
		if (ret < 0) {
			perror("fhi_lookup");
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
		if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
			(void )fprintf(stderr,
					   "%s: handle data too large\n",
					   component_name);
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getattr)(&sysio_fp_handle, &stbuf);
	if (ret) {
		perror("fhi_getattr");
		
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}

	/*
	 * If this is not a directory, use unlink to remove the data
	 */
	if(S_ISDIR(stbuf.st_mode))
	{
		if (full_path)
		{
			where.fhida_path = full_path;
			where.fhida_dir = &zoidfs_sysio_root_handle;
		}
		else {
			where.fhida_path = component_name;
			where.fhida_dir = &sysio_parent_handle;
		}
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_rmdir)(&where);
		if (ret < 0)
		{
			fprintf(stderr, "zoidfs_sysio_remove: fhi_rmdir() failed, code = %i.\n", ret);
			perror("zoidfs_sysio_remove");
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	/*
	 * Else, this is a directory, so use rmdir
	 */
	else
	{
		if (full_path)
		{
			where.fhida_path = full_path;
			where.fhida_dir = &zoidfs_sysio_root_handle;
		}
		else
		{
			where.fhida_path = component_name;
			where.fhida_dir = &sysio_parent_handle;
		}
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_unlink)(&where);
		if (ret < 0)
		{
			fprintf(stderr, "zoidfs_sysio_remove: fhi_unlink() failed, code = %i.\n", ret);
			perror("zoidfs_sysio_remove");
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
static int zoidfs_sysio_commit(const zoidfs_handle_t * UNUSED(handle)) {
	ZFSSYSIO_TRACE_ENTER;
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_create
 * This function creates a new file.
 */
static int zoidfs_sysio_create(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
                  int *created) {
	
    int ret;
    struct file_handle_info_dirop_args where;

	ZFSSYSIO_TRACE_ENTER;
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_create: Invalid path parameters.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

    if (full_path)
	{
		int ret = 0;
		char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};

		/*
		 * The file exists... don't create it and return a misc err code
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path, 0, &sysio_component_handle);
		if(ret >= 0)
		{
            *created = 0;
			sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
			ZFSSYSIO_TRACE_EXIT;
            return ZFS_OK;
		}
		
        where.fhida_path = full_path;
		where.fhida_dir = &zoidfs_sysio_root_handle;

        ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_create)(&where, sattr->mode);
        if (ret < 0) {
            fprintf(stderr, "zoidfs_sysio_create: fhi_create() failed, full path = %s.\n", where.fhida_path);
			perror("zoidfs_sysio_create");
            *created = 0;
            return sysio_err_to_zfs_err(errno);
        }
		
		/*
		 * get the handle for the file just created
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path, 0, &sysio_component_handle);
		if(ret >= 0)
		{
			*created = 1;
			sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
			ZFSSYSIO_TRACE_EXIT;
            return ZFS_OK;
		}
		sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
		
	/*
	 * Not the full path, so build the root handle
	 */
    } else {
		char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
		char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
		
		/*
		 * Setup the file handle based on the libsysio handle and the file handle
		 */
		zoidfs_handle_to_sysio_handle(parent_handle, &sysio_parent_handle);
		
		/*
		 * The file exists... don't create it and return a misc err code
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&sysio_parent_handle, component_name, 0, &sysio_component_handle);
		if(ret >= 0)
		{
            *created = 0;
			sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
			ZFSSYSIO_TRACE_EXIT;
            return ZFS_OK;
		}
		where.fhida_path = component_name;
		where.fhida_dir = &sysio_parent_handle;
    
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_create)(&where, sattr->mode);
        if (ret < 0)
		{
            fprintf(stderr, "zoidfs_sysio_create: fhi_create() failed, comp path = %s.\n", where.fhida_path);
			perror("zoidfs_sysio_create");
            *created = 0;
			ZFSSYSIO_TRACE_EXIT;
            return sysio_err_to_zfs_err(errno);
		}
		/*
		 * get the handle for the file just created
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&sysio_parent_handle, component_name, 0, &sysio_component_handle);
		if(ret >= 0)
		{
			*created = 1;
			sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
			ZFSSYSIO_TRACE_EXIT;
            return ZFS_OK;
		}
		sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
    }
    *created = 1;
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_rename
 * This function renames an existing file or directory.
 */
static int zoidfs_sysio_rename(const zoidfs_handle_t *from_parent_handle,
                  const char *from_component_name,
                  const char *from_full_path,
                  const zoidfs_handle_t *to_parent_handle,
                  const char *to_component_name,
                  const char *to_full_path,
                  zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                  zoidfs_cache_hint_t * UNUSED(to_parent_hint)) {
	
    int ret;
    struct file_handle_info_dirop_args where_to;
	struct file_handle_info_dirop_args where_from;
	char sysio_to_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_to_parent_handle = {NULL, sysio_to_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	char sysio_from_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_from_parent_handle = {NULL, sysio_from_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_sysio_rename: Invalid path parameters.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Build the where_from and where_to structs based on the given zoidfs data
	 */
	if(from_full_path)
	{
        where_from.fhida_path = from_full_path;
		where_from.fhida_dir = &zoidfs_sysio_root_handle;
	}
	if(to_full_path)
	{
        where_to.fhida_path = to_full_path;
		where_to.fhida_dir = &zoidfs_sysio_root_handle;
	}
	if(from_component_name)
	{
		zoidfs_handle_to_sysio_handle(from_parent_handle, &sysio_from_parent_handle);
		where_from.fhida_path = from_component_name;
		where_from.fhida_dir = &sysio_from_parent_handle;
	}
	if(to_component_name)
	{
		zoidfs_handle_to_sysio_handle(to_parent_handle, &sysio_to_parent_handle);
        where_to.fhida_path = to_component_name;
		where_to.fhida_dir = &sysio_to_parent_handle;
	}

	/*
	 * Invoke the libsysio rename call
	 */
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_rename)(&where_from, &where_to);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_rename: fhi_rename() failed.\n");
		perror("zoidfs_sysio_rename");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_link
 * This function creates a hard link.
 */
static int zoidfs_sysio_link(const zoidfs_handle_t *from_parent_handle,
                const char *from_component_name,
                const char *from_full_path,
                const zoidfs_handle_t *to_parent_handle,
                const char *to_component_name,
                const char *to_full_path,
                zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                zoidfs_cache_hint_t * UNUSED(to_parent_hint)) {
	
    int ret;
    struct file_handle_info_dirop_args where_to;
	struct file_handle_info_dirop_args where_from;
	char sysio_to_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_to_parent_handle = {NULL, sysio_to_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	char sysio_from_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_from_parent_handle = {NULL, sysio_from_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_sysio_rename: Invalid path parameters.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Build the where_from and where_to structs based on the given zoidfs data
	 */
	if(from_full_path)
	{
        where_from.fhida_path = from_full_path;
		where_from.fhida_dir = &zoidfs_sysio_root_handle;
	}
	if(to_full_path)
	{
        where_to.fhida_path = to_full_path;
		where_to.fhida_dir = &zoidfs_sysio_root_handle;
	}
	if(from_component_name)
	{
		zoidfs_handle_to_sysio_handle(from_parent_handle, &sysio_from_parent_handle);
		where_from.fhida_path = from_component_name;
		where_from.fhida_dir = &sysio_from_parent_handle;
	}
	if(to_component_name)
	{
		zoidfs_handle_to_sysio_handle(to_parent_handle, &sysio_to_parent_handle);
        where_to.fhida_path = to_component_name;
		where_to.fhida_dir = &sysio_to_parent_handle;
	}
	
	/*
	 * Invoke the libsysio link call
	 */
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_link)(&where_to, &where_from);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_link: fhi_link() failed, code = %i.\n", ret);
		perror("zoidfs_sysio_link");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_symlink
 * This function creates a symbolic link.
 */
static int zoidfs_sysio_symlink(const zoidfs_handle_t *from_parent_handle,
                   const char *from_component_name,
                   const char *from_full_path,
                   const zoidfs_handle_t *to_parent_handle,
                   const char *to_component_name,
                   const char *to_full_path,
                   const zoidfs_sattr_t * UNUSED(sattr),
                   zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                   zoidfs_cache_hint_t * UNUSED(to_parent_hint)) {
	
    int ret;
    struct file_handle_info_dirop_args where_to;
	struct file_handle_info_dirop_args where_from;
	char sysio_to_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_to_parent_handle = {NULL, sysio_to_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	char sysio_from_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_from_parent_handle = {NULL, sysio_from_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	const char * path_from;
	const char * path_to;
	
	ZFSSYSIO_TRACE_ENTER;
	
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_sysio_symlink: Invalid path parameters.\n");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Build the where_from and where_to structs based on the given zoidfs data
	 */
	if(to_full_path)
	{
        where_to.fhida_path = to_full_path;
		where_to.fhida_dir = &zoidfs_sysio_root_handle;
		path_to = to_full_path;
	}
	if(to_component_name)
	{
		zoidfs_handle_to_sysio_handle(to_parent_handle, &sysio_to_parent_handle);
		where_to.fhida_path = to_component_name;
		where_to.fhida_dir = &sysio_to_parent_handle;
		path_to = to_component_name;
	}
	if(from_full_path)
	{
        where_from.fhida_path = from_full_path;
		where_from.fhida_dir = &zoidfs_sysio_root_handle;
		path_from = from_full_path;
	}
	if(from_component_name)
	{
		zoidfs_handle_to_sysio_handle(from_parent_handle, &sysio_from_parent_handle);
		where_from.fhida_path = from_component_name;
		where_from.fhida_dir = &sysio_from_parent_handle;
		path_from = from_component_name;
	}

	/*
	 * Invoke the libsysio symlink call
	 */
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_symlink)(&where_from, path_to);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_symlink: fhi_symlink() failed.\n");
		perror("zoidfs_sysio_symlink");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	ZFSSYSIO_TRACE_EXIT;
	return ZFS_OK;
}


/*
 * zoidfs_mkdir
 * This function creates a new directory.
 */
static int zoidfs_sysio_mkdir(const zoidfs_handle_t *parent_handle,
                 const char *component_name, const char *full_path,
                 const zoidfs_sattr_t *sattr,
                 zoidfs_cache_hint_t * UNUSED(parent_hint)) {
	
    int ret;
    struct file_handle_info_dirop_args where;
	ZFSSYSIO_TRACE_ENTER;
	
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_sysio_mkdir: Invalid path parameters.\n");
		perror("zoidfs_sysio_mkdir");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

    if (full_path)
	{
		/*
		 * Assume the base path to export is "/"
		 */
		int ret = 0;
		
        where.fhida_path = full_path;
		where.fhida_dir = &zoidfs_sysio_root_handle;

        ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_mkdir)(&where, sattr->mode);
        if (ret < 0) {
            fprintf(stderr, "zoidfs_sysio_mkdir: fhi_mkdir() failed, code = %i.\n", ret);
			perror("zoidfs_sysio_mkdir");
			ZFSSYSIO_TRACE_EXIT;
            return sysio_err_to_zfs_err(errno);
        }
	/*
	 * Not the full path, so build the root handle
	 */
    } else {
		char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
		
		/*
		 * Setup the file handle based on the libsysio handle and the file handle
		 */
		zoidfs_handle_to_sysio_handle(parent_handle, &sysio_parent_handle);
	
		where.fhida_path = component_name;
		where.fhida_dir = &sysio_parent_handle;
    
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_mkdir)(&where, sattr->mode);
        if (ret < 0)
		{
            fprintf(stderr, "zoidfs_sysio_mkdir: fhi_mkdir() failed, code = %i.\n", ret);
			perror("zoidfs_sysio_mkdir");
			ZFSSYSIO_TRACE_EXIT;
            return sysio_err_to_zfs_err(errno);
		}
    }
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_readdir
 * This function returns the dirents from the specified parent directory. The
 * cookie is a pointer which specifies where in the directory to start
 * fetching the dirents from.
 */
static int zoidfs_sysio_readdir(const zoidfs_handle_t *parent_handle,
                   zoidfs_dirent_cookie_t cookie, size_t *entry_count,
                   zoidfs_dirent_t * UNUSED(entries), uint32_t UNUSED(flags),
                   zoidfs_cache_hint_t * UNUSED(parent_hint)) {

	char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	struct dirent64 * buffer, * dp;
    int cc = 0;
	
	ZFSSYSIO_TRACE_ENTER;
	/*
	 * Malloc the dirent buffer
	 */
	buffer = (struct dirent64 *)malloc(*entry_count * sizeof(struct dirent64));
	
	/*
     * Setup the file handle based on the libsysio handle and the file handle
     */
	zoidfs_handle_to_sysio_handle(parent_handle, &sysio_parent_handle);
	
	while ((cc = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getdirentries64)(&sysio_parent_handle, (char *)buffer, *entry_count, (off64_t *) &cookie)) > 0) {
		dp = buffer;
		while (cc > 0) {
			(void )fprintf(stderr, "\t%s: ino %llu type %u\n",
				      dp->d_name,
				      (unsigned long long )dp->d_ino,
				      (int )dp->d_type);
			cc -= dp->d_reclen;
			/* dp = (struct dirent *)((char *)dp + dp->d_reclen); */
		}
	}
	
	if(cc < 0)
	{
		fprintf(stderr, "zoidfs_sysio_readdir: fhi_getdirents64() failed, code = %i\n", cc);
		perror("zoidfs_sysio_readdir");
		ZFSSYSIO_TRACE_EXIT;
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_resize
 * This function resizes the file associated with the file handle.
 */
static int zoidfs_sysio_resize(const zoidfs_handle_t *handle, uint64_t size)
{
	
    /*
     * Setup the file handle based on the libsysio handle and the file handle
     */
	char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
    int ret = 0;

	zoidfs_handle_to_sysio_handle(handle, &sysio_component_handle);

	ZFSSYSIO_TRACE_ENTER;

	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_truncate)(&sysio_component_handle, size);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_resize: fhi_truncate() failed, code = %i.\n", ret);
		perror("zoidfs_sysio_resize");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}

/*
 * zoidfs_write
 * This function implements the zoidfs write call.
 */
static int zoidfs_sysio_write(const zoidfs_handle_t *handle, size_t mem_count,
                 const void *mem_starts[], const size_t mem_sizes[],
                 size_t file_count, const uint64_t file_starts[],
                 uint64_t file_sizes[]) {
	
	int ret = 0;
	unsigned int i = 0;
	struct iovec * iovs = (struct iovec *)malloc(sizeof(struct iovec) * mem_count);
	struct xtvec64 * xtvs = (struct xtvec64 *)malloc(sizeof(struct xtvec64) * mem_count);
	char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
	ioid_t ioidp;
	
	ZFSSYSIO_TRACE_ENTER;
	
	/*
	 * setup the iovec (memory) data structure
	 */
	for(i = 0 ; i < mem_count ; i+=1)
	{
		iovs[i].iov_base = (void *)mem_starts[i];
		iovs[i].iov_len = (size_t)mem_sizes[i];
	}
	
	/*
	 * setup the xtvec (file) data structure
	 */
	for(i = 0 ; i < file_count ; i+=1)
	{
		xtvs[i].xtv_off = (__off64_t)file_starts[i];
		xtvs[i].xtv_len = (size_t)file_sizes[i];
	}
		
    /*
     * Setup the file handle based on the libsysio handle and the file handle
     */
	zoidfs_handle_to_sysio_handle(handle, &sysio_component_handle);
	
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iwrite64x)(&sysio_component_handle, iovs, mem_count, xtvs, file_count, &ioidp);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_write: fhi_iwrite64x() failed, code = %i.\n", ret);
		perror("zoidfs_sysio_write");

	    free(iovs);
	    free(xtvs);

		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
		
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iowait)(&ioidp);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_write: fhi_iowait() failed, code = %i.\n", ret);
		perror("zoidfs_sysio_write");

	    free(iovs);
	    free(xtvs);

		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	/*
	 * Cleanup the data structures
	 */
	free(iovs);
	free(xtvs);
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
static int zoidfs_sysio_read(const zoidfs_handle_t *handle, size_t mem_count,
                void *mem_starts[], const size_t mem_sizes[],
                size_t file_count, const uint64_t file_starts[],
                uint64_t file_sizes[]) {
	
	int ret = 0;
	unsigned int i = 0;
	struct iovec * iovs = (struct iovec *)malloc(sizeof(struct iovec) * mem_count);
	struct xtvec64 * xtvs = (struct xtvec64 *)malloc(sizeof(struct xtvec64) * mem_count);
	char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
	ioid_t ioidp;

	ZFSSYSIO_TRACE_ENTER;
	
	/*
	 * setup the iovec (memory) data structure
	 */
	for(i = 0 ; i < mem_count ; i+=1)
	{
		iovs[i].iov_base = (void *)mem_starts[i];
		iovs[i].iov_len = (size_t)mem_sizes[i];
	}
	
	/*
	 * setup the xtvec (file) data structure
	 */
	for(i = 0 ; i < file_count ; i+=1)
	{
		xtvs[i].xtv_off = (__off64_t)file_starts[i];
		xtvs[i].xtv_len = (size_t)file_sizes[i];
	}
		
    /*
     * Setup the file handle based on the libsysio handle and the file handle
     */
	zoidfs_handle_to_sysio_handle(handle, &sysio_component_handle);
	
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iread64x)(&sysio_component_handle, iovs, mem_count, xtvs, file_count, &ioidp);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_read: fhi_iread64x() failed, code = %i.\n", ret);
		perror("zoidfs_sysio_read");
	
        free(iovs);
	    free(xtvs);

		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
		
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iowait)(&ioidp);
	if (ret < 0) {
		fprintf(stderr, "zoidfs_sysio_read: fhi_iowait() failed, code = %i.\n", ret);
		perror("zoidfs_sysio_write");
	
        free(iovs);
	    free(xtvs);

		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
	/*
	 * Cleanup the data structures
	 */
	free(iovs);
	free(xtvs);
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}

/*
 * Init all the drivers we know about.
 */
static int zoidfs_sysio_drv_init_all()
{
	extern int _sysio_native_init(void);
	extern int _sysio_incore_init(void);
	
	/*
	 * Only use the native sysio driver
	 */
	int (*drvinits[])(void) = {
			_sysio_native_init,
			NULL
	};
	
	int (**f)(void);
	int	err;

	ZFSSYSIO_TRACE_ENTER;

	err = 0;
	f = drvinits;
	while (*f) {
		err = (**f++)();
		if (err)
		{
			fprintf(stderr, "zoidfs_sysio_drv_init_all: failed, code = %i\n", err);
			perror("zoidfs_sysio_drv_init_all");
			ZFSSYSIO_TRACE_EXIT;
			return err;
		}
	}
	
	ZFSSYSIO_TRACE_EXIT;
	return 0;
}

/*
 * zoidfs_init
 */
static int zoidfs_sysio_init(void) {
	
	ZFSSYSIO_TRACE_ENTER;
    /* Initialize SYSIO */

	
	pthread_mutex_lock(&sysio_init_mutex);
    sysio_dispatcher_ref_count++;	
	if(!sysio_dispatcher_initialized)
	{
		char * arg = NULL;
		int err = _sysio_init();
		int mfs_key = 1;
		int root_key = 2;
		char * mfs = getenv("ZOIDFS_SYSIO_MOUNT");
		if (err)
		{
			fprintf(stderr, "zoidfs_sysio_init() failed\n");
			ZFSSYSIO_TRACE_EXIT;
			return err;
		}
	
		err = zoidfs_sysio_drv_init_all();
		if (err)
		{
			fprintf(stderr, "zoidfs_sysio_init() failed\n");
			ZFSSYSIO_TRACE_EXIT;
			return err;
		}
	
		/*
		 * Setup the sysio namespace and mounts... setup native mount
		 * at /
		 */
		arg = "{mnt,dev=\"native:/\",dir=/,fl=2}";
		err = _sysio_boot("namespace", arg);
		if (err)
		{
			fprintf(stderr, "zoidfs_sysio_init() failed");
			ZFSSYSIO_TRACE_EXIT;
			return err;
		}
	
		/*
		 * Export the root and mounted fs
		 */
		strcpy(zoidfs_sysio_mfs_path, mfs);
		err = zoidfs_sysio_export(&mfs_key, mfs, &zoidfs_sysio_mfs_handle);
		err = zoidfs_sysio_rootof(&zoidfs_sysio_mfs_handle);
	
		strcpy(zoidfs_sysio_root_path, "/");
		err = zoidfs_sysio_export(&root_key, "/", &zoidfs_sysio_root_handle);
		err = zoidfs_sysio_rootof(&zoidfs_sysio_root_handle);
		
		sysio_dispatcher_initialized = 1;
	}
	pthread_mutex_unlock(&sysio_init_mutex);
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_finalize
 */
static int zoidfs_sysio_finalize(void) {
	
	ZFSSYSIO_TRACE_ENTER;
	
	pthread_mutex_lock(&sysio_init_mutex);
	/* Finalize SYSIO */
	
	/*
	 * The sysio cleanup calls trigger asserts
	 * on mutex_destroy calls within the sysio lib...
	 *
	 * Disable the cleanup code until this is
	 * corrected
	 */
    if(sysio_dispatcher_initialized && sysio_dispatcher_ref_count == 1)
    {
	    zoidfs_sysio_unexport(&zoidfs_sysio_root_handle);
	    zoidfs_sysio_unexport(&zoidfs_sysio_mfs_handle);
        _sysio_shutdown();
        sysio_dispatcher_initialized = 0;
    }
    else
    {
        sysio_dispatcher_ref_count--;
    }
	
	pthread_mutex_unlock(&sysio_init_mutex);
	
	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}

static int zoidfs_sysio_resolve_path(const char * local_path,
                                            char * fs_path,
                                            int fs_path_max,
                                            zoidfs_handle_t * newhandle,
                                            int * usenew) 
{
	ZFSSYSIO_TRACE_ENTER;
	*usenew = 0; 

	/*
	 * If the fs_path buffer is available, identify the full path
	 */
	if (fs_path)
	{
		strncpy(fs_path, local_path, fs_path_max);
		fs_path[fs_path_max - 1] = '\0';
	}
	/*
	 * If the handle buffer is available, copy the correct handle
	 */
	if(newhandle)
	{
		sysio_handle_to_zoidfs_handle(&zoidfs_sysio_root_handle, newhandle);
		*usenew = 1;
	}
	
	ZFSSYSIO_TRACE_EXIT;
	return ZFS_OK;
}

/*
 * Function pointers for the sysio library
 */
zint_handler_t sysio_handler = {
   zoidfs_sysio_null,
   zoidfs_sysio_getattr,
   zoidfs_sysio_setattr,
   zoidfs_sysio_lookup,
   zoidfs_sysio_readlink,
   zoidfs_sysio_read,
   zoidfs_sysio_write,
   zoidfs_sysio_commit,
   zoidfs_sysio_create,
   zoidfs_sysio_remove,
   zoidfs_sysio_rename,
   zoidfs_sysio_link,
   zoidfs_sysio_symlink,
   zoidfs_sysio_mkdir,
   zoidfs_sysio_readdir,
   zoidfs_sysio_resize,
   zoidfs_sysio_resolve_path,
   zoidfs_sysio_init,
   zoidfs_sysio_finalize
};

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
