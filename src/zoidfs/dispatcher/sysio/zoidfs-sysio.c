/*
 * zoidfs-sysio.c
 * SYSIO driver for the ZOIDFS API.
 *
 * Jason Cope <copej@mcs.anl.gov>
 * Nawab Ali <alin@cse.ohio-state.edu>
 * 
 */

#include <sys/time.h>
#include "zoidfs-sysio.h"

#define SYSIO_WR_IO_CS
#define SYSIO_RD_IO_CS
/*
 * libsysio FHI function declarations externs for alternate symbol builds
 */

extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_export)(const void *key, size_t keylen, const char *source, unsigned flags, struct file_handle_info_export **fhiexpp);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_unexport)(struct file_handle_info_export *fhiexp);
extern ssize_t SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_root_of)(struct file_handle_info_export *fhiexp, struct file_handle_info *fhi);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iodone)(ioid_t ioid);
extern ssize_t SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iowait)(ioid_t ioid);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getattr)(struct file_handle_info *fhi, struct stat64 *buf);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_setattr)(struct file_handle_info *fhi, struct file_handle_info_sattr *fhisattr);
/*extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_truncate)(struct file_handle_info * fhi, off64_t size);*/
extern ssize_t SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(struct file_handle_info *parent_fhi, const char *path, unsigned iopmask, struct file_handle_info *result);
extern ssize_t SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_readlink)(struct file_handle_info *fhi, char *buf, size_t bufsiz);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iread64x)(struct file_handle_info *fhi, const struct iovec *iov, size_t iov_count, const struct xtvec64 *xtv, size_t xtv_count, ioid_t *ioidp);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iwrite64x)(struct file_handle_info *fhi, const struct iovec *iov, size_t iov_count, const struct xtvec64 *xtv, size_t xtv_count, ioid_t *ioidp);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_create)(struct file_handle_info_dirop_args *where, mode_t mode);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_unlink)(struct file_handle_info_dirop_args *where);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_rename)(struct file_handle_info_dirop_args *from, struct file_handle_info_dirop_args *to);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_link)(struct file_handle_info_dirop_args *from, struct file_handle_info_dirop_args *to);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_symlink)(const char * from, struct file_handle_info_dirop_args *to);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_mkdir)(struct file_handle_info_dirop_args *where, mode_t mode);
extern int SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_rmdir)(struct file_handle_info_dirop_args *where);
extern ssize_t SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getdirentries64)(struct file_handle_info *fhi, char *buf, size_t nbytes, off64_t * __restrict basep);
/*
 * libsysio init and shutdown function externs
 */
extern int _sysio_init(void); 
extern int _sysio_shutdown(void);
extern int _sysio_boot(char *, char *);
 
/*
 * zoidfs sysio init variables
 * ... make sure we only init once
 */
static int sysio_dispatcher_initialized = 0;
static int sysio_dispatcher_ref_count = 0;
static pthread_mutex_t sysio_init_mutex = PTHREAD_MUTEX_INITIALIZER;
#ifdef SYSIO_WR_IO_CS
static pthread_mutex_t sysio_wr_io_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#ifdef SYSIO_RD_IO_CS
static pthread_mutex_t sysio_rd_io_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

/*
 * root zoidfs handle
 */
static char zoidfs_sysio_root_path[4096];
static char zoidfs_sysio_root_handle_data[SYSIO_HANDLE_DATA_SIZE];
static struct file_handle_info zoidfs_sysio_root_handle = {NULL, zoidfs_sysio_root_handle_data, SYSIO_HANDLE_DATA_SIZE};

static char zfs_sysio_driver[32];
/*
 * zoidfs sysio trace and debug tools
 */

 
/*#define ZFSSYSIO_DEBUG_ENABLED
#define ZFSSYSIO_TRACE_ENABLED*/


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

#ifdef ZFSSYSIO_DEBUG_ENABLED
#define ZFSSYSIO_DEBUG(...) \
    do { \
        char __buffer[4096]; \
        sprintf(__buffer, ##__VA_ARGS__); \
        fprintf(stderr, "%s %s, ZOIDFS SYSIO DISPATCHER - DEBUG %s() %s:%i : %s\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__, __buffer); \
    }while(0)

#define ZFSSYSIO_INFO(...) \
    do { \
        char __buffer[4096]; \
        sprintf(__buffer, ##__VA_ARGS__); \
        fprintf(stderr, "%s %s, ZOIDFS SYSIO DISPATCHER - INFO %s() %s:%i : %s\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__, __buffer); \
    }while(0)

#define ZFSSYSIO_PERROR(...) \
    do { \
        char __buffer[4096]; \
        char __ebuffer[2048]; \
        strerror_r(errno, __ebuffer, 2048); \
        sprintf(__buffer, ##__VA_ARGS__); \
        fprintf(stderr, "%s %s, ZOIDFS SYSIO DISPATCHER - ERROR %s() %s:%i : %s, %s\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__, __buffer, __ebuffer); \
    }while(0)
#else
#define ZFSSYSIO_DEBUG(...) /* trace disabled */
#define ZFSSYSIO_INFO(...) /* trace disabled */
#define ZFSSYSIO_PERROR(...) /* trace disabled */

#endif


/* 
 * determine the static size of print storage buffers for the handles
*/
#ifndef  _ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE
#define  _ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE (2 * ZOIDFS_HANDLE_PAYLOAD_SIZE + 1)
#endif /*  _ZOIDFS_SYSIO_THANDLE_BUFFER_SIZE */

static inline char * zoidfs_sysio_trim_root_path(const char * path)
{
    /* if the driver is the native driver, trim the paths */
    if(strcmp(zfs_sysio_driver, "native") == 0)
    {
        if(strncmp(path, zoidfs_sysio_root_path, strlen(zoidfs_sysio_root_path)) == 0)
        {
            /* if we are looking for the roor path */
            if(strlen(path) == strlen(zoidfs_sysio_root_path))
            {
                return "/";
            }
            return (char *) &path[strlen(zoidfs_sysio_root_path)];
        }
        return (char *) path;
    }
    /* else, return the path as is */
    return (char *) path;
}

/*
 * Make sure that the full path is an absolute path
 */
static inline int zoidfs_sysio_valid_full_path(const char * path)
{
    if(path[0] == '/')
    {
        return 1;
    }
    return 0;
}

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
    ZFSSYSIO_INFO("zfs handle data: %s", vstr);
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
    ZFSSYSIO_INFO("sysio handle data: %s", vstr);
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
        ZFSSYSIO_INFO("zoidfs-sysio handle cmp, handles are equal");
    }
    else
    {
        ZFSSYSIO_INFO("zoidfs-sysio handle cmp, handles are not equal");
        if(memcmp(v, zoidfs_handle->data + ZOIDFS_HANDLE_HEADER_SIZE, SYSIO_FHE_SIZE) != 0)
        {
            ZFSSYSIO_INFO("zoidfs-sysio handle cmp, fhe_export differs");
        }
        if(memcmp(v + SYSIO_FHE_SIZE, zoidfs_handle->data + ZOIDFS_HANDLE_HEADER_SIZE + SYSIO_FHE_SIZE, SYSIO_HANDLE_DATA_SIZE) != 0)
        {
            ZFSSYSIO_INFO("zoidfs-sysio handle cmp, fhi_handle differs");
        }
        if(memcmp(v + SYSIO_FHE_SIZE + SYSIO_HANDLE_DATA_SIZE, zoidfs_handle->data + ZOIDFS_HANDLE_HEADER_SIZE + SYSIO_FHE_SIZE + SYSIO_HANDLE_DATA_SIZE, SYSIO_FHILENPACK_SIZE) != 0)
        {
            ZFSSYSIO_INFO("zoidfs-sysio handle cmp, fh_handle_len differs");
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
    ZFSSYSIO_INFO("s2z handle data");  
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
    ZFSSYSIO_INFO("z2s handle data");  
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
    
    /* check the format of the path */
    if(!zoidfs_sysio_valid_full_path(base_path))
    {
	    ZFSSYSIO_INFO("zoidfs_sysio_export: invalid full path format.");
		ZFSSYSIO_TRACE_EXIT;
        return ZFSERR_OTHER; 
    }

    /*
     * export remote_fs_path based on key
     */
    ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_export)(key, sizeof(*key), base_path, 0, &(root_handle->fhi_export));
    if (ret) {
	    ZFSSYSIO_INFO("zoidfs_sysio_export: fhi_export() failed. key = %i, base_path = %s", *key, base_path);
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
		ZFSSYSIO_INFO("zoidfs_sysio_unexport: fhi_unexport() failed.");
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
	ZFSSYSIO_INFO("zoidfs_sysio_rootof: fhi_root_of() failed.");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
    /*
     * Verify that the handle data is not too large
     */
    if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
	ZFSSYSIO_INFO("zoidfs_sysio_rootof: handle data too large, %lu >= %lu", (unsigned long)ret, (unsigned long)root_handle->fhi_handle_len);
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
    static char sysio_handle_data[SYSIO_HANDLE_DATA_SIZE];
    struct file_handle_info sysio_handle = {NULL, (void *)sysio_handle_data, SYSIO_HANDLE_DATA_SIZE};
    int smask = 0;

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
        ZFSSYSIO_INFO("zoidfs_sysio_getattr: fhi_getattr() failed, code = %i.", ret);
		ZFSSYSIO_PERROR("zoidfs_sysio_getattr");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

    /* Convert the SYSIO attributes to ZOIDFS attributes */
    if((attr->mask & ZOIDFS_ATTR_MODE) == ZOIDFS_ATTR_MODE)
    {
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
        
        attr->mode = stbuf.st_mode & 0777;
        smask = smask | ZOIDFS_ATTR_MODE;
    }

    if((attr->mask & ZOIDFS_ATTR_NLINK) == ZOIDFS_ATTR_NLINK)
    {
        attr->nlink = stbuf.st_nlink;
        smask = smask | ZOIDFS_ATTR_NLINK;
    }
    if((attr->mask & ZOIDFS_ATTR_UID) == ZOIDFS_ATTR_UID)
    {
        attr->uid = stbuf.st_uid;
        smask = smask | ZOIDFS_ATTR_UID;
    }
    if((attr->mask & ZOIDFS_ATTR_GID) == ZOIDFS_ATTR_GID)
    {
        attr->gid = stbuf.st_gid;
        smask = smask | ZOIDFS_ATTR_GID;
    }
    if((attr->mask & ZOIDFS_ATTR_SIZE) == ZOIDFS_ATTR_SIZE)
    {
        attr->size = stbuf.st_size;
        smask = smask | ZOIDFS_ATTR_SIZE;
    }
    if((attr->mask & ZOIDFS_ATTR_BSIZE) == ZOIDFS_ATTR_BSIZE)
    {
        attr->blocksize = stbuf.st_blksize;
        smask = smask | ZOIDFS_ATTR_BSIZE;
    }
    if((attr->mask & ZOIDFS_ATTR_ATIME) == ZOIDFS_ATTR_ATIME)
    {
        attr->atime.seconds = stbuf.st_atime;
        attr->atime.nseconds = 0;
        smask = smask | ZOIDFS_ATTR_ATIME;
    }
    if((attr->mask & ZOIDFS_ATTR_MTIME) == ZOIDFS_ATTR_MTIME)
    {
        attr->mtime.seconds = stbuf.st_mtime;
        attr->mtime.nseconds = 0;
        smask = smask | ZOIDFS_ATTR_MTIME;
    }
    if((attr->mask & ZOIDFS_ATTR_CTIME) == ZOIDFS_ATTR_CTIME)
    {
        attr->ctime.seconds = stbuf.st_ctime;
        attr->ctime.nseconds = 0;
        smask = smask | ZOIDFS_ATTR_CTIME;
    }
    if((attr->mask & ZOIDFS_ATTR_FSID) == ZOIDFS_ATTR_FSID)
    {
        attr->fsid = stbuf.st_dev;
        smask = smask | ZOIDFS_ATTR_FSID;
    }
    if((attr->mask & ZOIDFS_ATTR_FILEID) == ZOIDFS_ATTR_FILEID)
    {
        attr->fileid = stbuf.st_ino;
        smask = smask | ZOIDFS_ATTR_FILEID;
    }

    /* reset the mask with the values set in attr */
    attr->mask = smask;

	ZFSSYSIO_TRACE_EXIT;
    return ZFS_OK;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
static int zoidfs_sysio_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t *attr) {
	
    int ret = 0;
    struct file_handle_info_sattr sysio_sattr;

	int setAttrs = 0;
    static char sysio_handle_data[SYSIO_HANDLE_DATA_SIZE];
    struct file_handle_info sysio_handle = {NULL, sysio_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;

	/*
	 * Convert the zoidfs handle to a sysio handle
	 * and initialize the handle
	 */ 
	zoidfs_handle_to_sysio_handle(handle, &sysio_handle);

    /* erase the attr */	
    memset(&sysio_sattr, 0, sizeof(sysio_sattr));

	/*
	 * Set the mode
	 */
	if(sattr->mask & ZOIDFS_ATTR_MODE)
	{
		sysio_sattr.fhisattr_mode = sattr->mode;
		sysio_sattr.fhisattr_mode_set = 1;
		setAttrs = 1;
		ZFSSYSIO_DEBUG("zoidfs_sysio_setattr: set mode = %o",  sysio_sattr.fhisattr_mode);
	}
	/*
	 * Set the UID
	 */
	if(sattr->mask & ZOIDFS_ATTR_UID)
	{
		sysio_sattr.fhisattr_uid = sattr->uid;
		sysio_sattr.fhisattr_uid_set = 1;
		setAttrs = 1;
		ZFSSYSIO_DEBUG("zoidfs_sysio_setattr: set uid = %i",  sysio_sattr.fhisattr_uid);
	}
	/*
	 * Set the GID
	 */
	if(sattr->mask & ZOIDFS_ATTR_GID)
	{
		sysio_sattr.fhisattr_gid = sattr->gid;
		sysio_sattr.fhisattr_gid_set = 1;
		setAttrs = 1;
		ZFSSYSIO_DEBUG("zoidfs_sysio_setattr: set gid = %i",  sysio_sattr.fhisattr_gid);
	}
	/*
	 * Set the MTIME
	 */
	if(sattr->mask & ZOIDFS_ATTR_MTIME)
	{
		sysio_sattr.fhisattr_mtime = sattr->mtime.seconds;
		sysio_sattr.fhisattr_mtime_set = 1;
		setAttrs = 1;
		ZFSSYSIO_DEBUG("zoidfs_sysio_setattr: set mtime = %lu",  sysio_sattr.fhisattr_mtime);
	}
	/*
	 * Set the ATIME
	 */
	if(sattr->mask & ZOIDFS_ATTR_ATIME)
	{
		sysio_sattr.fhisattr_atime = sattr->atime.seconds;
		sysio_sattr.fhisattr_atime_set = 1;
		setAttrs = 1;
		ZFSSYSIO_DEBUG("zoidfs_sysio_setattr: set atime = %lu",  sysio_sattr.fhisattr_atime);
	}
	/*
	 * Set the SIZE
	 */
	if(sattr->mask & ZOIDFS_ATTR_SIZE)
	{
		sysio_sattr.fhisattr_size = sattr->size;
		sysio_sattr.fhisattr_size_set = 1;
		setAttrs = 1;
		ZFSSYSIO_DEBUG("zoidfs_sysio_setattr: set size = %lu",  sysio_sattr.fhisattr_size);
	}
	
	/*
	 * Execute the sysio setattr call
	 */
	if(setAttrs)
	{
        
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_setattr)(&sysio_handle, &sysio_sattr);
		if (ret) {
			ZFSSYSIO_INFO("zoidfs_sysio_setattr: fhi_setattr() failed.");
			ZFSSYSIO_PERROR("zoidfs_sysio_setattr");
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	
	/*
	 * Execute the zoidfs_sysio getattr call and populate the attr struct
     * attr can be NULL... only run getattr if not NULL
	 */
    if(attr)
    {
	    ret = zoidfs_sysio_getattr(handle, attr);
	    if (ret != ZFS_OK) 
        {
		    ZFSSYSIO_INFO("zoidfs_sysio_setattr: fhi_getattr() failed.");
		    ZFSSYSIO_PERROR("zoidfs_sysio_setattr");
		    ZFSSYSIO_TRACE_EXIT;
		    return sysio_err_to_zfs_err(errno);
	    }
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
	static char sysio_link_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_link_handle = {NULL, sysio_link_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;
	
	zoidfs_handle_to_sysio_handle(handle, &sysio_link_handle);

    /* init the buffer to NULL chars */
    memset(buffer, 0, buffer_length);

	/*
	 * Is this a link?
	 */
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getattr)(&sysio_link_handle, &stbuf);
	if (ret) {
		ZFSSYSIO_PERROR("fhi_getattr");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}

	if (S_ISLNK(stbuf.st_mode)) {
        char intnl_buffer[ZOIDFS_PATH_MAX];
        int intnl_buffer_length = ZOIDFS_PATH_MAX + 1; 
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_readlink)(&sysio_link_handle, intnl_buffer, intnl_buffer_length);
		if (ret < 0) {
			ZFSSYSIO_PERROR("readlink");
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}

        /* make the buffer NULL terminated */
        intnl_buffer[intnl_buffer_length - 1] = '\0';

        /* if the root mount point is included on the return path, remove the mnt pt */
        if(strncmp(intnl_buffer, zoidfs_sysio_root_path, strlen(zoidfs_sysio_root_path)) == 0)
        {
            char * _t_buffer = strdup(intnl_buffer);
            strcpy(intnl_buffer, &_t_buffer[strlen(zoidfs_sysio_root_path)]);
            intnl_buffer[intnl_buffer_length - 1 ] = '\0';
            free(_t_buffer);
        }

        /* copy intnl_buffer into the user buffer */
        memset(buffer, 0, buffer_length);
        strcpy(buffer, intnl_buffer);
	}
    else
    {
        ZFSSYSIO_INFO("zoidfs_sysio_readlink: invalid operation, not a link");
	    ZFSSYSIO_TRACE_EXIT;
        return ZFSERR_OTHER;
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
        ZFSSYSIO_INFO("zoidfs_sysio_lookup: Invalid path parameters.");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Full path given
	 */
    if (full_path) 
    {
		int ret = 0;
        char * full_path_trim = zoidfs_sysio_trim_root_path(full_path);
		/*
		 * sysio parent handle
		 */
		static char h_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info h = {NULL, h_data, SYSIO_HANDLE_DATA_SIZE};

        if(!full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_lookup: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
	
        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_lookup: invalid full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

		/*
		 * Lookup the exported file info for the parent handle
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path_trim, 0, &h);
		if(ret < 0)
		{
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
		if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
			ZFSSYSIO_INFO("zoidfs_sysio_rootof: handle data too large, %lu >= %lu", (unsigned long)ret, (unsigned long)zoidfs_sysio_root_handle.fhi_handle_len);
			ZFSSYSIO_INFO("handle data too large");
		}
		sysio_handle_to_zoidfs_handle(&h, handle);

	/*
	 * Else, parent handle and component_name given
	 */
    } else {
		/*
		 * sysio parent handle
		 */
		static char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
		
		/*
		 * sysio component handle
		 */
		static char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
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
			ZFSSYSIO_INFO("zoidfs_sysio_rootof: handle data too large, %i >= %i", ret, SYSIO_HANDLE_DATA_SIZE);
			ZFSSYSIO_INFO("handle data too large");
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
    static char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
    struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
    struct file_handle_info_dirop_args where;
	static char sysio_fp_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_fp_handle = {NULL, sysio_fp_handle_data, SYSIO_HANDLE_DATA_SIZE};
	struct stat64 stbuf;
    char * full_path_trim = NULL;

	ZFSSYSIO_TRACE_ENTER;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        ZFSSYSIO_INFO("zoidfs_sysio_remove: Invalid path parameters.");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
	
	if(full_path)
	{
		int ret = 0;

        full_path_trim = zoidfs_sysio_trim_root_path(full_path);

        if(!full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_lookup: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
	
        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_remove: invalid full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
	
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path_trim, 0, &sysio_fp_handle);
		if (ret < 0) {
			ZFSSYSIO_PERROR("fhi_lookup");
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
		if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
			ZFSSYSIO_INFO("%s: handle data too large", full_path);
			
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
			ZFSSYSIO_PERROR("fhi_lookup");
            ZFSSYSIO_INFO("fhi_lookup failed: component_name = %s", component_name);
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
		if ((size_t )ret >= SYSIO_HANDLE_DATA_SIZE) {
			ZFSSYSIO_INFO("%s: handle data too large", component_name);
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}

    /* get the handle attrs... beware of STALE */
    int stale_count = 0;
    do
    {
	    ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getattr)(&sysio_fp_handle, &stbuf);
        /* if we got an ESTALE, attempt to revlaidate the handle */    
        if(ret == ESTALE)
        {
            int _ret = 0;
            _ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&sysio_parent_handle, component_name, 0, &sysio_fp_handle);
            if(_ret != 0)
            {
			    ZFSSYSIO_PERROR("fhi_lookup");
                ZFSSYSIO_INFO("fhi_lookup / handle revalidate failed: component_name = %s", component_name);
			    ZFSSYSIO_TRACE_EXIT;
			    return sysio_err_to_zfs_err(errno);
            }
        }
        stale_count++;
    }while((ret == ESTALE || ret == -ESTALE || errno == ESTALE || errno == -ESTALE) && stale_count <= 1);

	if (ret) {
		ZFSSYSIO_PERROR("fhi_getattr");
		
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}

	/*
	 * If this is not a directory, use unlink to remove the data
	 */
	if(S_ISDIR(stbuf.st_mode))
	{
		if (full_path_trim)
		{
			where.fhida_path = full_path_trim;
			where.fhida_dir = &zoidfs_sysio_root_handle;
		}
		else {
			where.fhida_path = component_name;
			where.fhida_dir = &sysio_parent_handle;
		}
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_rmdir)(&where);
		if (ret < 0)
		{
			ZFSSYSIO_INFO("zoidfs_sysio_remove: fhi_rmdir() failed, code = %i.", ret);
			ZFSSYSIO_PERROR("zoidfs_sysio_remove");
			
			ZFSSYSIO_TRACE_EXIT;
			return sysio_err_to_zfs_err(errno);
		}
	}
	/*
	 * Else, this is a directory, so use rmdir
	 */
	else
	{
		if (full_path_trim)
		{
			where.fhida_path = full_path_trim;
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
			ZFSSYSIO_INFO("zoidfs_sysio_remove: fhi_unlink() failed, code = %i.", ret);
			ZFSSYSIO_PERROR("zoidfs_sysio_remove");
			
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
    zoidfs_sattr_t local_sattr;
    zoidfs_attr_t local_attr;

	ZFSSYSIO_TRACE_ENTER;

    memset(&local_sattr, 0, sizeof(local_sattr));
    memset(&local_attr, 0, sizeof(local_attr));

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        ZFSSYSIO_INFO("zoidfs_create: Invalid path parameters.");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

    /* If the mask is not set, populate with current values */
    /* 
        TODO: This should be improved... get uid / gid info from
        the client, get the default mode / umask info from the 
        the client...
    */
    if(!sattr->mask)
    {
        struct timeval now;
        local_sattr.mask = ZOIDFS_ATTR_SETABLE;
        local_sattr.mode = 0644;
        local_sattr.uid = getuid();
        local_sattr.gid = getgid();
        gettimeofday(&now, NULL);
        local_sattr.atime.seconds = now.tv_sec;
        local_sattr.atime.nseconds = 0;
        local_sattr.mtime.seconds = now.tv_sec;
        local_sattr.mtime.nseconds = 0;
    }
    else
    {
        local_sattr.mask = sattr->mask;
        local_sattr.mode = sattr->mode & 0777;
        local_sattr.uid = sattr->uid;
        local_sattr.gid = sattr->gid;
        local_sattr.atime.seconds = sattr->atime.seconds;
        local_sattr.atime.nseconds = 0;
        local_sattr.mtime.seconds = sattr->mtime.seconds;
        local_sattr.mtime.nseconds = 0;
    }

    if (full_path)
	{
		int ret = 0;
		static char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
        char * full_path_trim = zoidfs_sysio_trim_root_path(full_path);

        if(!full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_lookup: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_create: invalid full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

		/*
		 * The file exists... don't create it, set attrs, and return the handle w/ created == 0 
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path_trim, 0, &sysio_component_handle);
		if(ret >= 0)
		{
            *created = 0;

			sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);

            ret = zoidfs_sysio_setattr(handle, &local_sattr, &local_attr);
            if(ret != ZFS_OK)
            {
                ZFSSYSIO_INFO("zoidfs_sysio_create: zoidfs_sysio_setattr() failed");
                ZFSSYSIO_PERROR("zoidfs_sysio_create");
                return sysio_err_to_zfs_err(errno);
            }

			ZFSSYSIO_TRACE_EXIT;
            return ZFS_OK;
		}
		
        where.fhida_path = full_path_trim;
		where.fhida_dir = &zoidfs_sysio_root_handle;

        ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_create)(&where, local_sattr.mode);
        if (ret < 0) {
            ZFSSYSIO_INFO("zoidfs_sysio_create: fhi_create() failed, full path = %s.", full_path);
			ZFSSYSIO_PERROR("zoidfs_sysio_create");
            *created = 0;
            return sysio_err_to_zfs_err(errno);
        }
		
		/*
		 * get the handle for the file just created
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&zoidfs_sysio_root_handle, full_path_trim, 0, &sysio_component_handle);
		if(ret >= 0)
		{
			*created = 1;
			sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
			ZFSSYSIO_TRACE_EXIT;
            return ZFS_OK;
		}
		sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);
		
        /*
         * set the file attributes
         */
        ret = zoidfs_sysio_setattr(handle, &local_sattr, &local_attr);
        if(ret != ZFS_OK)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_create: zoidfs_sysio_setattr() failed");
			ZFSSYSIO_PERROR("zoidfs_sysio_create");
            return sysio_err_to_zfs_err(errno);
        }

	/*
	 * Not the full path, so build the root handle
	 */
    } else {
		static char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
		static char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
		struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
		
		/*
		 * Setup the file handle based on the libsysio handle and the file handle
		 */
		zoidfs_handle_to_sysio_handle(parent_handle, &sysio_parent_handle);
		
		/*
		 * The file exists... don't create it, set the attrs, and return a misc err code
		 */
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_lookup)(&sysio_parent_handle, component_name, 0, &sysio_component_handle);
		if(ret >= 0)
		{
            *created = 0;
			sysio_handle_to_zoidfs_handle(&sysio_component_handle, handle);

            ret = zoidfs_sysio_setattr(handle, &local_sattr, &local_attr);
            if(ret != ZFS_OK)
            {
                ZFSSYSIO_INFO("zoidfs_sysio_create: zoidfs_sysio_setattr() failed");
                ZFSSYSIO_PERROR("zoidfs_sysio_create");
                return sysio_err_to_zfs_err(errno);
            }

			ZFSSYSIO_TRACE_EXIT;
            return ZFS_OK;
		}
		where.fhida_path = component_name;
		where.fhida_dir = &sysio_parent_handle;
    
		ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_create)(&where, local_sattr.mode);
        if (ret < 0)
		{
            ZFSSYSIO_INFO("zoidfs_sysio_create: fhi_create() failed, comp path = %s.", where.fhida_path);
			ZFSSYSIO_PERROR("zoidfs_sysio_create");
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

        /*
         * set the file attributes
         */
        /* since we can't revalidate the parent handle, return ESTALE and do nothing */
        //ret = zoidfs_sysio_setattr(handle, &local_sattr, &local_attr);
        ret = 0;
        if(ret != ZFS_OK)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_create: zoidfs_sysio_setattr() failed");
			ZFSSYSIO_PERROR("zoidfs_sysio_create");
            return sysio_err_to_zfs_err(errno);
        }
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
	static char sysio_to_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_to_parent_handle = {NULL, sysio_to_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	static char sysio_from_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_from_parent_handle = {NULL, sysio_from_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        ZFSSYSIO_INFO("zoidfs_sysio_rename: Invalid path parameters.");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Build the where_from and where_to structs based on the given zoidfs data
	 */
	if(from_full_path)
	{
        char * from_full_path_trim = zoidfs_sysio_trim_root_path(from_full_path);

        if(!from_full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_rename: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(from_full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_rename: invalid from full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
        where_from.fhida_path = from_full_path_trim;
		where_from.fhida_dir = &zoidfs_sysio_root_handle;
	}
	if(to_full_path)
	{
        char * to_full_path_trim = zoidfs_sysio_trim_root_path(to_full_path);

        if(!to_full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_rename: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(to_full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_rename: invalid to full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
        where_to.fhida_path = to_full_path_trim;
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
		ZFSSYSIO_INFO("zoidfs_sysio_rename: fhi_rename() failed.");
		ZFSSYSIO_PERROR("zoidfs_sysio_rename");
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
	static char sysio_to_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_to_parent_handle = {NULL, sysio_to_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	static char sysio_from_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_from_parent_handle = {NULL, sysio_from_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	
	ZFSSYSIO_TRACE_ENTER;
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        ZFSSYSIO_INFO("zoidfs_sysio_rename: Invalid path parameters.");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Build the where_from and where_to structs based on the given zoidfs data
	 */
	if(from_full_path)
	{
        char * from_full_path_trim = zoidfs_sysio_trim_root_path(from_full_path);

        if(!from_full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_link: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(from_full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_link: invalid from full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
        where_from.fhida_path = from_full_path_trim;
		where_from.fhida_dir = &zoidfs_sysio_root_handle;
	}
	if(to_full_path)
	{
        char * to_full_path_trim = zoidfs_sysio_trim_root_path(to_full_path);
        zoidfs_handle_t fhandle;

        if(!to_full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_link: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        /* If the file exists, return an error */        
        if(zoidfs_lookup(NULL, NULL, to_full_path, &fhandle) == ZFS_OK)
        {
		    ZFSSYSIO_INFO("zoidfs_sysio_link: target file already exists");
            return sysio_err_to_zfs_err(EEXIST);
        }

        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(to_full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_link: invalid to full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
        where_to.fhida_path = to_full_path_trim;
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
        zoidfs_handle_t fhandle;

        /* If the file exists, return an error */        
        if(zoidfs_lookup(to_parent_handle, to_component_name, NULL, &fhandle) == ZFS_OK)
        {
		    ZFSSYSIO_INFO("zoidfs_sysio_link: target file already exists");
            return sysio_err_to_zfs_err(EEXIST);
        }

		zoidfs_handle_to_sysio_handle(to_parent_handle, &sysio_to_parent_handle);
        where_to.fhida_path = to_component_name;
		where_to.fhida_dir = &sysio_to_parent_handle;
	}
	
	/*
	 * Invoke the libsysio link call
	 */
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_link)(&where_from, &where_to);
	if (ret < 0) {
		ZFSSYSIO_INFO("zoidfs_sysio_link: fhi_link() failed, code = %i.", ret);
		ZFSSYSIO_PERROR("zoidfs_sysio_link");
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
	static char sysio_to_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_to_parent_handle = {NULL, sysio_to_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	static char sysio_from_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_from_parent_handle = {NULL, sysio_from_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	const char * path_from = NULL;
	const char * path_to = NULL;
    char new_link_path[ZOIDFS_PATH_MAX];
	
	ZFSSYSIO_TRACE_ENTER;
	
    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        ZFSSYSIO_INFO("zoidfs_sysio_symlink: Invalid path parameters.");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

	/*
	 * Build the where_from and where_to structs based on the given zoidfs data
	 */
	if(to_full_path)
	{
        zoidfs_handle_t fhandle;
        char * to_full_path_trim = zoidfs_sysio_trim_root_path(to_full_path);

        if(!to_full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_symlink: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        /* If the file exists, return an error */        
        if(zoidfs_lookup(NULL, NULL, to_full_path, &fhandle) == ZFS_OK)
        {
		    ZFSSYSIO_INFO("zoidfs_sysio_symlink: target file already exists");
            return sysio_err_to_zfs_err(EEXIST);
        }

        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(to_full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_symlink: invalid to full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
        where_to.fhida_path = to_full_path_trim;
		where_to.fhida_dir = &zoidfs_sysio_root_handle;
		path_to = to_full_path_trim;
	}
	if(to_component_name)
	{
        zoidfs_handle_t fhandle;

        /* If the file exists, return an error */        
        if(zoidfs_lookup(to_parent_handle, to_component_name, NULL, &fhandle) == ZFS_OK)
        {
		    ZFSSYSIO_INFO("zoidfs_sysio_symlink: target file already exists");
            return sysio_err_to_zfs_err(EEXIST);
        }

		zoidfs_handle_to_sysio_handle(to_parent_handle, &sysio_to_parent_handle);
		where_to.fhida_path = to_component_name;
		where_to.fhida_dir = &sysio_to_parent_handle;
		path_to = to_component_name;
	}
	if(from_full_path)
	{
        char * from_full_path_trim = zoidfs_sysio_trim_root_path(from_full_path);

        if(!from_full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_symlink: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(from_full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_symlink: invalid from full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
        sprintf(new_link_path, "%s%s", zoidfs_sysio_root_path, from_full_path_trim);
        where_from.fhida_path = new_link_path;
		where_from.fhida_dir = &zoidfs_sysio_root_handle;
		path_from = new_link_path;
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
	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_symlink)((const char *)path_from, &where_to);
	if (ret < 0) {
		ZFSSYSIO_INFO("zoidfs_sysio_symlink: fhi_symlink() failed.");
		ZFSSYSIO_PERROR("zoidfs_sysio_symlink");
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
        ZFSSYSIO_INFO("zoidfs_sysio_mkdir: Invalid path parameters.");
		ZFSSYSIO_PERROR("zoidfs_sysio_mkdir");
		ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }

    if (full_path)
	{
        char * full_path_trim = zoidfs_sysio_trim_root_path(full_path);
		/*
		 * Assume the base path to export is "/"
		 */
		int ret = 0;

        if(!full_path_trim)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_mkdir: invalid full path.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }
	
        /* check the format of the path */
        if(!zoidfs_sysio_valid_full_path(full_path_trim))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_mkdir: invalid full path format.");
            ZFSSYSIO_TRACE_EXIT;
            return ZFSERR_OTHER;
        }

        where.fhida_path = full_path_trim;
		where.fhida_dir = &zoidfs_sysio_root_handle;

        ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_mkdir)(&where, sattr->mode);
        if (ret < 0) {
            ZFSSYSIO_INFO("zoidfs_sysio_mkdir: fhi_mkdir() failed, code = %i, full_path = %s, full_path_trim = %s.", ret, full_path, full_path_trim);
			ZFSSYSIO_PERROR("zoidfs_sysio_mkdir");
			ZFSSYSIO_TRACE_EXIT;
            return sysio_err_to_zfs_err(errno);
        }
	/*
	 * Not the full path, so build the root handle
	 */
    } else {
		static char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
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
            ZFSSYSIO_INFO("zoidfs_sysio_mkdir: fhi_mkdir() failed, code = %i.", ret);
			ZFSSYSIO_PERROR("zoidfs_sysio_mkdir");
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
                   zoidfs_dirent_t * entries, uint32_t flags,
                   zoidfs_cache_hint_t * UNUSED(parent_hint)) {

    struct stat64 stbuf;
	static char sysio_parent_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_parent_handle = {NULL, sysio_parent_handle_data, SYSIO_HANDLE_DATA_SIZE};
	struct dirent64 * dp;
    static char buf[4096];
    int cc = 0;
    int ret = 0;
    unsigned int i = 0;	
    off64_t base = cookie;
    off64_t t_base = cookie;

	ZFSSYSIO_TRACE_ENTER;

	/*
     * Setup the file handle based on the libsysio handle and the file handle
     */
	zoidfs_handle_to_sysio_handle(parent_handle, &sysio_parent_handle);

    /*
     * determine if this is a dir... if not, exit with an error
     */
    ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getattr)(&sysio_parent_handle, &stbuf);
    if (ret) {
        ZFSSYSIO_PERROR("fhi_getattr");

        ZFSSYSIO_TRACE_EXIT;
        return sysio_err_to_zfs_err(errno);
    }
    if(!S_ISDIR(stbuf.st_mode))
    {
        ZFSSYSIO_INFO("zoidfs_sysio_readdir: invalid operation, handle not a directory");
	    ZFSSYSIO_TRACE_EXIT;
        return ZFSERR_NOTDIR;
    }

    /*
     * Look for all directory entries in the handle
     */
	for(;;) 
    {
        cc = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_getdirentries64)(&sysio_parent_handle, (char *)buf, sizeof(buf), (off64_t *) &base);

        /*
         * If we recieved no more dir entries, break from the loop and return the number of entries found
         */
        if(cc <= 0)
        {
            *entry_count = i;
            break;
        }
		
        /*
         * Parse the raw dir entry data and convert it to a zoidfs readable format
         */
        dp = (struct dirent64 *) buf;
        t_base = base;
		while (cc > 0 && i < *entry_count)
        {
            zoidfs_attr_t attr;

            /*
             * Lookup the entry handle and save it if ZOIDFS_RETR_HANDLE
             */
            if((flags & ZOIDFS_RETR_HANDLE) == ZOIDFS_RETR_HANDLE)
            {
                /*
                 * lookup and save the handle
                 */
                zoidfs_lookup(parent_handle, dp->d_name, NULL, &entries[i].handle);

                /*
                 * Get the handle attrs
                 */
                attr.mask = ZOIDFS_ATTR_ALL;
                zoidfs_getattr(&entries[i].handle, &entries[i].attr);
            }
            else
            {
                zoidfs_handle_t ehandle;
                zoidfs_lookup(parent_handle, dp->d_name, NULL, &ehandle);

                /*
                 * Get the handle attrs
                 */
                memset(&attr, 0, sizeof(attr));
                memset(&entries[i].attr, 0, sizeof(entries[i].attr));
                attr.mask = ZOIDFS_ATTR_ALL;
                zoidfs_getattr(&ehandle, &entries[i].attr);
            }

            /*
             * Copy sysio dirents into zfs dirents 
             */
            strncpy(entries[i].name, dp->d_name, ZOIDFS_NAME_MAX + 1);
            entries[i].cookie = t_base;
            /* ZFSSYSIO_INFO("dir ent name = %s cookie = %lu off = %lu reclen = %lu", entries[i].name, entries[i].cookie, dp->d_off, dp->d_reclen); */

            i++; 
            t_base += dp->d_reclen; 
            cc -= dp->d_reclen;
			dp = (struct dirent64 *)((char *)dp + dp->d_reclen);
		}
       
        /*
         * No more entries allowed... exit loop
         */ 
        if(i == *entry_count)
        {
            break;
        }
	}

    /* ZFSSYSIO_INFO("entries found = %i", *entry_count); */
	if(cc < 0)
	{
		ZFSSYSIO_INFO("zoidfs_sysio_readdir: fhi_getdirents64() failed, code = %i", cc);
		ZFSSYSIO_PERROR("zoidfs_sysio_readdir");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}

    /* For now, assume that this completes correctly */
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

    /* using the fho_truncate method */
	/*static char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
    int ret = 0;

	ZFSSYSIO_TRACE_ENTER;
	zoidfs_handle_to_sysio_handle(handle, &sysio_component_handle);

    ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_truncate)(&sysio_component_handle, size);
    if(ret != ZFS_OK)
    {
		ZFSSYSIO_INFO("zoidfs_sysio_resize: fhi_truncate() failed, code = %i.", ret);
		ZFSSYSIO_PERROR("zoidfs_sysio_resize");
		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
    }*/

    /* using setattr method */
    int ret = 0;
    zoidfs_sattr_t sattr;
    zoidfs_attr_t attr;

    /* make sure the handle refers to a regular file */
    memset(&attr, 0, sizeof(attr));
    attr.mask = ZOIDFS_ATTR_MODE;

    zoidfs_getattr(handle, &attr);

    if(attr.type == ZOIDFS_REG)
    {
        memset(&sattr, 0, sizeof(sattr));
        sattr.mask = ZOIDFS_ATTR_SIZE;
        sattr.size = size;
    
        ret = zoidfs_sysio_setattr(handle, &sattr, NULL);
    }
    else
    {
		ZFSSYSIO_INFO("zoidfs_sysio_resize: fhi_setattr() failed, cannot set the size of a non-regular file");
	    ZFSSYSIO_TRACE_EXIT;
        return ZFSERR_INVAL;
    }

	ZFSSYSIO_TRACE_EXIT;
    return ret;
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
	struct iovec * iovs = NULL;
	struct xtvec64 * xtvs = NULL;
	static char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
	ioid_t ioidp;
    uint64_t total_size = 0;
    uint64_t io_total_size = 0;
	
	ZFSSYSIO_TRACE_ENTER;

    /* nothing in the memory buffers or the file buffers, no work to do */
    if(mem_count == 0 || file_count == 0)
    {
		ZFSSYSIO_TRACE_EXIT;
        return ZFS_OK;
    }

    iovs = (struct iovec *)malloc(sizeof(struct iovec) * mem_count);
    if(!iovs)
    {
        iovs = NULL;
		ZFSSYSIO_INFO("zoidfs_sysio_write: malloc() failed.");
		ZFSSYSIO_PERROR("zoidfs_sysio_write");
        return -ENOMEM;
    }

	xtvs = (struct xtvec64 *)malloc(sizeof(struct xtvec64) * file_count);
    if(!xtvs)
    {
        xtvs = NULL;
		ZFSSYSIO_INFO("zoidfs_sysio_write: malloc() failed.");
		ZFSSYSIO_PERROR("zoidfs_sysio_write");
        return -ENOMEM;
    }
	
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
        total_size += xtvs[i].xtv_len;
	}
		
    /*
     * Setup the file handle based on the libsysio handle and the file handle
     */
	zoidfs_handle_to_sysio_handle(handle, &sysio_component_handle);

#ifdef SYSIO_WR_IO_CS
    /* starting write io... lock the mutex */
	pthread_mutex_lock(&sysio_wr_io_mutex);
#endif

	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iwrite64x)(&sysio_component_handle, iovs, mem_count, xtvs, file_count, &ioidp);
	if (ret < 0) {
		ZFSSYSIO_INFO("zoidfs_sysio_write: fhi_iwrite64x() failed, code = %i.", ret);
		ZFSSYSIO_PERROR("zoidfs_sysio_write");

        if(iovs)
        {
	        free(iovs);
            iovs = NULL;
        }
        if(xtvs)
        {
	        free(xtvs);
            xtvs = NULL;
        }

#ifdef SYSIO_WR_IO_CS
        /* make sure to unlock the mutex */
	    pthread_mutex_unlock(&sysio_wr_io_mutex);
#endif

		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}

    /* poll until IO is done */
    do
    {
	    io_total_size = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iowait)(ioidp);
        if(total_size != io_total_size)
        {
		    ZFSSYSIO_INFO("zoidfs_sysio_write: io not done yet. %lu of %lu bytes completed.", io_total_size, total_size);
        }
        if(io_total_size == (size_t)(-EWOULDBLOCK))
        {
		    ZFSSYSIO_INFO("zoidfs_sysio_write: io not done yet... blocking.");
        }
    }while(io_total_size == (size_t)(-EWOULDBLOCK) && io_total_size != total_size);

#ifdef SYSIO_WR_IO_CS
    /* done with the write io... unlock the mutex */
	pthread_mutex_unlock(&sysio_wr_io_mutex);
#endif

	/*
	 * Cleanup the data structures
	 */
    if(iovs)
    {
	    free(iovs);
        iovs = NULL;
    }
    if(xtvs)
    {
	    free(xtvs);
        xtvs = NULL;
    }
	
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
	struct iovec * iovs = NULL;
	struct xtvec64 * xtvs = NULL;
	static char sysio_component_handle_data[SYSIO_HANDLE_DATA_SIZE];
	struct file_handle_info sysio_component_handle = {NULL, sysio_component_handle_data, SYSIO_HANDLE_DATA_SIZE};
	ioid_t ioidp;
    uint64_t total_size = 0;
    uint64_t io_total_size = 0;

	ZFSSYSIO_TRACE_ENTER;
	
    /* nothing in the memory buffers or the file buffers, no work to do */
    if(mem_count == 0 || file_count == 0)
    {
		ZFSSYSIO_TRACE_EXIT;
        return ZFS_OK;
    }

	iovs = (struct iovec *)malloc(sizeof(struct iovec) * mem_count);
    if(!iovs)
    {
        iovs = NULL;
		ZFSSYSIO_INFO("zoidfs_sysio_write: malloc() failed.");
		ZFSSYSIO_PERROR("zoidfs_sysio_write");
        return -ENOMEM;
    }

	xtvs = (struct xtvec64 *)malloc(sizeof(struct xtvec64) * file_count);
    if(!xtvs)
    {
        xtvs = NULL;
		ZFSSYSIO_INFO("zoidfs_sysio_write: malloc() failed.");
		ZFSSYSIO_PERROR("zoidfs_sysio_write");
        return -ENOMEM;
    }

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
        total_size += (xtvs[i].xtv_len);
	}
		
    /*
     * Setup the file handle based on the libsysio handle and the file handle
     */
	zoidfs_handle_to_sysio_handle(handle, &sysio_component_handle);

#ifdef SYSIO_RD_IO_CS
    /* starting read io... lock the mutex */
	pthread_mutex_lock(&sysio_rd_io_mutex);
#endif

	ret = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iread64x)(&sysio_component_handle, iovs, mem_count, xtvs, file_count, &ioidp);
	if (ret < 0) {
		ZFSSYSIO_INFO("zoidfs_sysio_read: fhi_iread64x() failed, code = %i.", ret);
		ZFSSYSIO_PERROR("zoidfs_sysio_read");

        if(iovs)
        {	
            free(iovs);
            iovs = NULL;
        }
        if(xtvs)
        {
	        free(xtvs);
            xtvs = NULL;
        }

#ifdef SYSIO_RD_IO_CS
        /* make sure to unlock the mutex */
	    pthread_mutex_unlock(&sysio_rd_io_mutex);
#endif

		ZFSSYSIO_TRACE_EXIT;
		return sysio_err_to_zfs_err(errno);
	}
		
    /* poll until IO is done */
    do
    {
	    io_total_size = SYSIO_INTERFACE_NAME(_zfs_sysio_fhi_iowait)(ioidp);
        if(total_size != io_total_size)
        {
            ZFSSYSIO_INFO("zoidfs_sysio_read: io not done yet. %lu of %lu bytes completed.", io_total_size, total_size);
        }
        if(io_total_size == (size_t)(-EWOULDBLOCK))
        {
            ZFSSYSIO_INFO("zoidfs_sysio_read: io not done yet... blocking.");
        }
    }while(io_total_size == (size_t)(-EWOULDBLOCK) && io_total_size != total_size);

#ifdef SYSIO_RD_IO_CS
    /* done with the read io... unlock the mutex */
	pthread_mutex_unlock(&sysio_rd_io_mutex);
#endif

	/*
	 * Cleanup the data structures
	 */
    if(iovs)
    {
	    free(iovs);
        iovs = NULL;
    }
    if(xtvs)
    {
	    free(xtvs);
        xtvs = NULL;
    }
	
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
			ZFSSYSIO_INFO("zoidfs_sysio_drv_init_all: failed, code = %i", err);
			ZFSSYSIO_PERROR("zoidfs_sysio_drv_init_all");
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
		char arg[256];
        int err = 0;
		int root_key = 1;
		char * mfs = getenv("ZOIDFS_SYSIO_MOUNT");
		char * mfs_root = getenv("ZOIDFS_SYSIO_MOUNT_ROOT");
		char * sysio_driver = getenv("ZOIDFS_SYSIO_DRIVER");

        if(!mfs)
        {
			ZFSSYSIO_INFO("zoidfs_sysio_init() failed, ZOIDFS_SYSIO_MOUNT env variable was not available");
			ZFSSYSIO_TRACE_EXIT;
			return err;
        }

        if(!sysio_driver)
        {
			ZFSSYSIO_INFO("zoidfs_sysio_init() failed, ZOIDFS_SYSIO_DRIVER env variable was not available");
			ZFSSYSIO_TRACE_EXIT;
			return err;
        }
        else
        {
            memset(zfs_sysio_driver, 0, 32);
            strcpy(zfs_sysio_driver, sysio_driver);
        }

		/*
		 * Setup the sysio namespace and mounts... setup native mount
		 * at /
         *
         * sysio driver are set using the ZOIDFS_SYSIO_DRIVER env
         * variable. Possible values include 'native', 'zoidfs', 'pvfs', and 'lustre'.
         *
		 */

        ZFSSYSIO_INFO("Attempting to mount %s with a root of %s using the libsysio %s driver", mfs_root, mfs, sysio_driver);
        if(strcmp(sysio_driver, "pvfs") == 0)
        {
            extern void start_pvfs_sysio_driver(char * m, char * mr);

            if(!mfs_root)
            {
			    ZFSSYSIO_INFO("zoidfs_sysio_init() failed, ZOIDFS_SYSIO_MOUNT_ROOT env variable was not available");
    			ZFSSYSIO_TRACE_EXIT;
	    		return err;
            }

            /* invoke the mount for the driver mount */
            start_pvfs_sysio_driver(mfs_root, mfs);
        }
        else if(strcmp(sysio_driver, "native") == 0)
        {
            err = _sysio_init();
            if(err)
            {
			    ZFSSYSIO_INFO("zoidfs_sysio_init() failed");
    			ZFSSYSIO_TRACE_EXIT;
	    		return err;
            }

		    err = zoidfs_sysio_drv_init_all();
		    if (err)
		    {
			    ZFSSYSIO_INFO("zoidfs_sysio_init() failed");
    			ZFSSYSIO_TRACE_EXIT;
	    		return err;
		    }

            /* setup the driver mount */	
		    sprintf(arg, "{mnt,dev=\"%s:%s\",dir=%s,fl=2}", sysio_driver, mfs_root, mfs);
            ZFSSYSIO_INFO("libsysio mount cmd = %s", arg);
		    err = _sysio_boot("namespace", arg);
		    if (err)
		    {
			    ZFSSYSIO_INFO("zoidfs_sysio_init() failed");
			    ZFSSYSIO_TRACE_EXIT;
		    	return err;
		    }
        }

        /* export the root handle */	
		strcpy(zoidfs_sysio_root_path, mfs_root);
		err = zoidfs_sysio_export(&root_key, mfs, &zoidfs_sysio_root_handle);
        if(err)
        {
			    ZFSSYSIO_INFO("zoidfs_sysio_init() failed: zoidfs_sysio_export() failed");
			    ZFSSYSIO_TRACE_EXIT;
		    	return err;
        }

		err = zoidfs_sysio_rootof(&zoidfs_sysio_root_handle);
        if(err)
        {
			    ZFSSYSIO_INFO("zoidfs_sysio_init() failed: zoidfs_sysio_rootof() failed");
			    ZFSSYSIO_TRACE_EXIT;
		    	return err;
        }
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
