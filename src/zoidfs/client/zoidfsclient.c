/*
 * zoidfs.c
 * Client-side implementation of the ZOIDFS API. The CNs communicate with
 * the IONs over BMI.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 * Jason Cope <copej@mcs.anl.gov>
 *    
 */

#include <bmi.h>
#include "zoidfs/zoidfs.h"
#include "bmi_comm.h"
#include "zoidfs_xdr.h"
#include "zoidfs/zoidfs-proto.h"

#include "c-util/tools.h"
#include <assert.h>
#include <pthread.h>

static char *ion_name;
static BMI_addr_t peer_addr;
static bmi_context_id context;

/* conditional compilation flags */
/*#define ZFS_USE_XDR_SIZE_CACHE
#define ZFS_BMI_FASTMEMALLOC*/

/*
 * profiling symbols for the zoidfs api
 */
#pragma weak zoidfs_getattr = Pzoidfs_getattr
#undef zoidfs_getattr
#define zoidfs_getattr Pzoidfs_getattr

#pragma weak zoidfs_setattr = Pzoidfs_setattr
#undef zoidfs_setattr
#define zoidfs_setattr Pzoidfs_setattr

#pragma weak zoidfs_lookup = Pzoidfs_lookup
#undef zoidfs_lookup
#define zoidfs_lookup Pzoidfs_lookup

#pragma weak zoidfs_readlink = Pzoidfs_readlink
#undef zoidfs_readlink
#define zoidfs_readlink Pzoidfs_readlink

#pragma weak zoidfs_read = Pzoidfs_read
#undef zoidfs_read
#define zoidfs_read Pzoidfs_read

#pragma weak zoidfs_write = Pzoidfs_write
#undef zoidfs_write
#define zoidfs_write Pzoidfs_write

#pragma weak zoidfs_commit = Pzoidfs_commit
#undef zoidfs_commit
#define zoidfs_commit Pzoidfs_commit

#pragma weak zoidfs_create = Pzoidfs_create
#undef zoidfs_create
#define zoidfs_create Pzoidfs_create

#pragma weak zoidfs_remove = Pzoidfs_remove
#undef zoidfs_remove
#define zoidfs_remove Pzoidfs_remove

#pragma weak zoidfs_rename = Pzoidfs_rename
#undef zoidfs_rename
#define zoidfs_rename Pzoidfs_rename

#pragma weak zoidfs_link = Pzoidfs_link
#undef zoidfs_link
#define zoidfs_link Pzoidfs_link

#pragma weak zoidfs_symlink = Pzoidfs_symlink
#undef zoidfs_symlink
#define zoidfs_symlink Pzoidfs_symlink

#pragma weak zoidfs_mkdir = Pzoidfs_mkdir
#undef zoidfs_mkdir
#define zoidfs_mkdir Pzoidfs_mkdir

#pragma weak zoidfs_readdir = Pzoidfs_readdir
#undef zoidfs_readdir
#define zoidfs_readdir Pzoidfs_readdir

#pragma weak zoidfs_resize = Pzoidfs_resize
#undef zoidfs_resize
#define zoidfs_resize Pzoidfs_resize

#pragma weak zoidfs_init = Pzoidfs_init
#undef zoidfs_init
#define zoidfs_init Pzoidfs_init

#pragma weak zoidfs_finalize = Pzoidfs_finalize
#undef zoidfs_finalize
#define zoidfs_finalize Pzoidfs_finalize

#pragma weak zoidfs_null = Pzoidfs_null
#undef zoidfs_null
#define zoidfs_null Pzoidfs_null

/* sizeof macros for zoidfs xdr data */
#define XDRSIZE_BUFFER_T(_len) xdr_stringsize (_len)
#define XDRSIZE_CSTRING_PATH_T(_cstring) xdr_stringsize (zfsmin(ZOIDFS_PATH_MAX + 1, strlen(_cstring)))
#define XDRSIZE_CSTRING_NAME_T(_cstring) xdr_stringsize (zfsmin(ZOIDFS_NAME_MAX + 1, strlen(_cstring)))

/* XDR stream wrapper data type*/
typedef struct
{
    XDR xdr;
    char xdr_init;
} zoidfs_xdr_t;

/* XDR Stream helper macros */
#define ZFS_XDR_DESTROY(_xdr)       \
do{                                 \
    if((_xdr).xdr_init == 1)        \
    {                               \
        xdr_destroy(&((_xdr).xdr)); \
        (_xdr).xdr_init = 0;        \
    }                               \
}while(0)
#define ZFS_XDR_INIT(_xdr) (_xdr).xdr_init = 0;
#define ZFS_XDRMEM_CREATE(_xdr, _sb, _sblen, _xdrflag) xdrmem_create(&((_xdr).xdr), (_sb), (unsigned int) (_sblen), (_xdrflag)); (_xdr).xdr_init = 1
#define ZFS_XDRSTREAM(_xdr) &(_xdr)->xdr
#define ZFS_XDRSTREAM_REF(_xdr) &(_xdr).xdr
/*
 * In cosidering the multi-threaded client (e.g. FUSE), we use different tag
 * for communication to identify the threads. This enables that bmi_post_recv()
 * receives the proper message which is heading to the caller's thread.
 */
static int gen_tag(void)
{
    return pthread_self();
}

static int zoidfs_write_pipeline(BMI_addr_t peer_addr, uint64_t pipeline_size,
                                 size_t list_count, const void ** buf_list,
                                 const bmi_size_t size_list[], bmi_msg_tag_t tag,
                                 bmi_context_id context);
static int zoidfs_read_pipeline(BMI_addr_t peer_addr, uint64_t pipeline_size,
                                size_t list_count, void ** buf_list,
                                const bmi_size_t size_list[], bmi_msg_tag_t tag,
                                bmi_context_id context);

/* 
 * zoidfs message data types
 * Used by the zoidfs xdr processor to encode / decode data
 */
typedef enum
{
    ZFS_OP_ID_T = 1,
    ZFS_OP_STATUS_T = 2,
    ZFS_HANDLE_T = 3,
    ZFS_ATTR_T = 4,
    ZFS_SATTR_T = 5,
    ZFS_NULL_PARAM_T = 6,
    ZFS_CACHE_HINT_T = 7,
    ZFS_DIRENT_COOKIE_T = 8,
    ZFS_UINT32_T = 9,
    ZFS_UINT64_T = 10,
    ZFS_UINT32_ARRAY_T = 11,
    ZFS_UINT64_ARRAY_T = 12,
    ZFS_INT_T = 13,
    ZFS_BUFFER_T = 14,
    ZFS_CSTRING_PATH_T = 15,
    ZFS_CSTRING_NAME_T = 16,
    ZFS_DIRENT_TRANSFER_T = 17,

    /* end of enum */
    ZFS_MSG_DATA_MAX = 18
} zoidfs_msg_data_t;

/* zoidfs data type wrappers for buffers and arrays */
typedef struct
{
    char * data;
    unsigned int len;
} zoidfs_path_transfer_t;

typedef struct
{
    char * data;
    unsigned int len;
} zoidfs_name_transfer_t;

typedef struct
{
    uint32_t * data;
    unsigned int len; 
} zoidfs_uint32_array_transfer_t;

typedef struct
{
    uint64_t * data;
    unsigned int len; 
} zoidfs_uint64_array_transfer_t;

typedef struct
{
    void * data;
    unsigned int len;
} zoidfs_buffer_transfer_t;

/*
 * xdr size processing for zoidfs messages
 */

#ifdef ZFS_USE_XDR_SIZE_CACHE
static unsigned int zoidfs_xdr_size_cache[ZFS_MSG_DATA_MAX];

static inline unsigned int zoidfs_xdr_size_processor_cache_init()
{
    zoidfs_msg_data_t i = 0;

    /* init the cache */
    for(i = 0 ; i < ZFS_MSG_DATA_MAX ; i++)
    {
        zoidfs_xdr_size_cache[i] = 0;

        switch(i)
        {
            case ZFS_OP_ID_T:
            {
                zoidfs_op_id_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &data);
                break;
            }
            case ZFS_OP_STATUS_T:
            {
                zoidfs_op_status_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &data);
                break;
            }
            case ZFS_HANDLE_T:
            {
                zoidfs_handle_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, &data);
                break;
            }
            case ZFS_ATTR_T:
            {
                zoidfs_attr_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_attr_t, &data);
                break;
            }
            case ZFS_SATTR_T:
            {
                zoidfs_sattr_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, &data);
                break;
            }
            case ZFS_NULL_PARAM_T:
            {   
                zoidfs_null_param_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t, &data);
                break;
            }
            case ZFS_CACHE_HINT_T:
            {
                zoidfs_cache_hint_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, &data);
                break;
            }
            case ZFS_DIRENT_COOKIE_T:
            {
                zoidfs_dirent_cookie_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_zoidfs_dirent_cookie_t, &data);
                break;
            }
            case ZFS_UINT32_T:
            {
                uint32_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_uint32_t, &data);
                break;
            }
            case ZFS_UINT64_T:
            {
                uint64_t data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_uint64_t, &data);
                break;
            }
            case ZFS_UINT32_ARRAY_T:
            {
                zoidfs_xdr_size_cache[i] = 0;
                break;
            }
            case ZFS_UINT64_ARRAY_T:
            {
                zoidfs_xdr_size_cache[i] = 0;
                break;
            }
            case ZFS_INT_T:
            {
                int data;
                zoidfs_xdr_size_cache[i] = xdr_sizeof((xdrproc_t)xdr_int, &data);
                break;
            }
            default:
            {
                zoidfs_xdr_size_cache[i] = 0;
            }
        }
    }

    return 0;
}
#endif

static inline unsigned int zoidfs_xdr_size_processor(zoidfs_msg_data_t data_t, const void * data)
{
    unsigned int size = 0; 
    if(data)
    {
#ifdef ZFS_USE_XDR_SIZE_CACHE
        size = zoidfs_xdr_size_cache[data_t];
#else
        switch(data_t)
        {
            case ZFS_OP_ID_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, (zoidfs_op_id_t *)data);
                break;
            }
            case ZFS_OP_STATUS_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, (zoidfs_op_status_t *)data);
                break;
            }
            case ZFS_HANDLE_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (zoidfs_handle_t *)data);
                break;
            }
            case ZFS_ATTR_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_attr_t, (zoidfs_sattr_t *)data);
                break;
            }
            case ZFS_SATTR_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, (zoidfs_sattr_t *)data);
                break;
            }
            case ZFS_NULL_PARAM_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t, (zoidfs_null_param_t *)data);
                break;
            }
            case ZFS_CACHE_HINT_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, (zoidfs_cache_hint_t *)data);
                break;
            }
            case ZFS_DIRENT_COOKIE_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_zoidfs_dirent_cookie_t, (zoidfs_dirent_cookie_t *)data);
                break;
            }
            case ZFS_UINT32_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_uint32_t, (uint32_t *)data);
                break;
            }
            case ZFS_UINT64_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_uint64_t, (uint64_t *)data);
                break;
            }
            case ZFS_UINT32_ARRAY_T:
            {
                size = sizeof(uint32_t) + (*(uint32_t *)data * (xdr_sizeof((xdrproc_t)xdr_uint32_t, (uint32_t *)data)));
                break;
            }
            case ZFS_UINT64_ARRAY_T:
            {
                size = sizeof(uint32_t) + (*(uint32_t *)data * (xdr_sizeof((xdrproc_t)xdr_uint64_t, (uint32_t *)data)));
                break;
            }
            case ZFS_INT_T:
            {
                size = xdr_sizeof((xdrproc_t)xdr_int, (int *)data);
                break;
            }
            default:
            {
                fprintf(stderr, "%s(): processing error, unknown zoidfs data type, %s:%i.\n", __func__, __FILE__, __LINE__);
                size = 0;
                break;
            }
        }
#endif
    }
    return size;
}

/*
 * xdr processing for zoidfs messages
 */
static inline int zoidfs_xdr_processor(zoidfs_msg_data_t data_t, void * data, zoidfs_xdr_t * xdr)
{
    zoidfs_op_status_t ret = ZFS_OK;

    switch(data_t)
    {
        case ZFS_OP_ID_T:
        {
            if (!xdr_zoidfs_op_id_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_op_id_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_OP_STATUS_T:
        {
            if (!xdr_zoidfs_op_status_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_op_status_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_HANDLE_T:
        {
            if (!xdr_zoidfs_handle_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_handle_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_ATTR_T:
        {
            if (!xdr_zoidfs_attr_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_attr_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_SATTR_T:
        {
            if (!xdr_zoidfs_sattr_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_sattr_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_NULL_PARAM_T:
        {
            if (!xdr_zoidfs_null_param_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_null_param_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_CACHE_HINT_T:
        {
            if (!xdr_zoidfs_cache_hint_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_cache_hint_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_DIRENT_COOKIE_T:
        {
            if (!xdr_zoidfs_dirent_cookie_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_dirent_cookie_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_UINT32_T:
        {
            if (!xdr_uint32_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_uint32_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_UINT64_T:
        {
            if (!xdr_uint64_t(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_uint64_t() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_INT_T:
        {
            if (!xdr_int(ZFS_XDRSTREAM((xdr)), data))
            {
                fprintf(stderr, "%s(): xdr_int() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_DIRENT_TRANSFER_T:
        {
            if(!xdr_zoidfs_dirent_array(ZFS_XDRSTREAM(xdr), data))
            {
                fprintf(stderr, "%s(): xdr_zoidfs_dirent_array() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_UINT32_ARRAY_T:
        {
            zoidfs_uint32_array_transfer_t * _data = (zoidfs_uint32_array_transfer_t *)data;
            if (!xdr_array(ZFS_XDRSTREAM(xdr), (char **)&_data->data, (unsigned int *)&_data->len, (unsigned int)_data->len, sizeof(uint32_t), (xdrproc_t)xdr_uint32_t))
            {
                fprintf(stderr, "%s(): xdr_array() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_UINT64_ARRAY_T:
        {
            zoidfs_uint64_array_transfer_t * _data = (zoidfs_uint64_array_transfer_t *)data;
            if (!xdr_array(ZFS_XDRSTREAM(xdr), (char **)&_data->data, (unsigned int *)&_data->len, (unsigned int)_data->len, sizeof(uint64_t), (xdrproc_t)xdr_uint64_t))
            {
                fprintf(stderr, "%s(): xdr_array() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_BUFFER_T:
        {
            zoidfs_buffer_transfer_t * _data = (zoidfs_buffer_transfer_t *)data;
            if(!xdr_string(ZFS_XDRSTREAM(xdr), (char **)&_data->data, _data->len))
            {
                fprintf(stderr, "%s(): xdr_string() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_CSTRING_PATH_T:
        {
            zoidfs_path_transfer_t * _data = (zoidfs_path_transfer_t *)data;
            if(!xdr_string(ZFS_XDRSTREAM(xdr), (char **)&_data->data, _data->len))
            {
                fprintf(stderr, "%s(): xdr_string() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        case ZFS_CSTRING_NAME_T:
        {
            zoidfs_name_transfer_t * _data = (zoidfs_name_transfer_t *)data;
            if(!xdr_string(ZFS_XDRSTREAM(xdr), (char **)&_data->data, _data->len))
            {
                fprintf(stderr, "%s(): xdr_string() failed, %s:%i.\n", __func__, __FILE__, __LINE__);
                ret = ZFSERR_XDR;
            }
            break;
        }
        default:
        {
            fprintf(stderr, "%s(): processing error, unknown zoidfs data type, %s:%i.\n", __func__, __FILE__, __LINE__);
            ret = ZFSERR_XDR;
            break;
        }
    }
    return ret;
}

typedef struct
{
    zoidfs_xdr_t send_xdr;
    void * sendbuf;
    bmi_size_t sendbuflen;
    bmi_msg_tag_t tag;

    zoidfs_op_id_t zoidfs_op_id;
} zoidfs_send_msg_t;

typedef struct
{
    zoidfs_xdr_t recv_xdr;
    void * recvbuf;
    bmi_size_t recvbuflen;
    bmi_msg_tag_t tag;
    bmi_size_t actual_size;

    zoidfs_op_status_t op_status;
} zoidfs_recv_msg_t;

#define ZOIDFS_RECV_MSG_INIT(_msg)      \
do{                                     \
    ZFS_XDR_INIT(_msg.recv_xdr);        \
    (_msg).op_status = ZFS_OK;          \
    (_msg).tag = gen_tag();             \
    (_msg).actual_size = 0;             \
    (_msg).recvbuflen = 0;              \
    (_msg).recvbuf = NULL;              \
}while(0)

#define ZOIDFS_SEND_MSG_INIT(_msg, _op_id)  \
do{                                         \
    ZFS_XDR_INIT(_msg.send_xdr);            \
    (_msg).zoidfs_op_id = _op_id;           \
    (_msg).tag = gen_tag();                 \
    (_msg).sendbuflen = 0;                  \
    (_msg).sendbuf = NULL;                  \
}while(0)

#ifdef ZFS_BMI_FASTMEMALLOC                
#define ZOIDFS_RECV_MSG_DESTROY(_msg)       \
do{                                         \
    ZFS_XDR_DESTROY((_msg).recv_xdr);       \
    if((_msg).recvbuf)                      \
    {                                       \
        (_msg).recvbuf = NULL;              \
        (_msg).recvbuflen = 0;              \
        (_msg).actual_size = 0;             \
    }                                       \
}while(0)
#else
#define ZOIDFS_RECV_MSG_DESTROY(_msg)       \
do{                                         \
    ZFS_XDR_DESTROY((_msg).recv_xdr);       \
    if((_msg).recvbuf)                      \
    {                                       \
        free((_msg).recvbuf);               \
        (_msg).recvbuf = NULL;              \
        (_msg).recvbuflen = 0;              \
        (_msg).actual_size = 0;             \
    }                                       \
}while(0)
#endif                                      

#ifdef ZFS_BMI_FASTMEMALLOC                
#define ZOIDFS_SEND_MSG_DESTROY(_msg)       \
do{                                         \
    ZFS_XDR_DESTROY((_msg).send_xdr);       \
    if((_msg).sendbuf)                      \
    {                                       \
        (_msg).sendbuf = NULL;              \
        (_msg).sendbuflen = 0;              \
    }                                       \
}while(0)
#else
#define ZOIDFS_SEND_MSG_DESTROY(_msg)       \
do{                                         \
    ZFS_XDR_DESTROY((_msg).send_xdr);       \
    if((_msg).sendbuf)                      \
    {                                       \
        free((_msg).sendbuf);               \
        (_msg).sendbuf = NULL;              \
        (_msg).sendbuflen = 0;              \
    }                                       \
}while(0)
#endif


/* reuse a static buffer */
#ifdef ZFS_BMI_FASTMEMALLOC
static void * zfs_bmi_client_sendbuf = NULL;
static void * zfs_bmi_client_recvbuf = NULL;
#define ZFS_BMI_CLIENT_SENDBUF_LEN 128 * 1024
#define ZFS_BMI_CLIENT_RECVBUF_LEN 128 * 1024
#define ZOIDFS_SEND_MSG_SET_BUFLEN(_msg, _val) (_msg).send_msg.sendbuflen = (_val)
#define ZOIDFS_RECV_MSG_SET_BUFLEN(_msg, _val) (_msg).recv_msg.recvbuflen = (_val)
#define ZOIDFS_SEND_ALLOC_BUFFER(_msg) (_msg).sendbuf = zfs_bmi_client_sendbuf
#define ZOIDFS_RECV_ALLOC_BUFFER(_msg) (_msg).recvbuf = zfs_bmi_client_recvbuf
/* allways alloc buffers */
#else
#define ZOIDFS_SEND_MSG_SET_BUFLEN(_msg, _val) (_msg).send_msg.sendbuflen = (_val)
#define ZOIDFS_RECV_MSG_SET_BUFLEN(_msg, _val) (_msg).recv_msg.recvbuflen = (_val)
#define ZOIDFS_SEND_ALLOC_BUFFER(_msg) (_msg).sendbuf = BMI_memalloc(peer_addr, (_msg).sendbuflen, BMI_SEND)
#define ZOIDFS_RECV_ALLOC_BUFFER(_msg) (_msg).recvbuf = BMI_memalloc(peer_addr, (_msg).recvbuflen, BMI_RECV)
#endif

#define ZOIDFS_SEND_XDR_MEMCREATE(_msg) ZFS_XDRMEM_CREATE((_msg).send_xdr, (_msg).sendbuf, (_msg).sendbuflen, XDR_ENCODE)
#define ZOIDFS_RECV_XDR_MEMCREATE(_msg) ZFS_XDRMEM_CREATE((_msg).recv_xdr, (_msg).recvbuf, (_msg).actual_size, XDR_DECODE)


#define ZOIDFS_BMI_COMM_SENDU(_msg) bmi_comm_sendu(peer_addr, (_msg).sendbuf, (_msg).sendbuflen, (_msg).tag, context)
#define ZOIDFS_BMI_COMM_RECV(_msg) bmi_comm_recv(peer_addr, (_msg).recvbuf, (_msg).recvbuflen, (_msg).tag, context, &(_msg).actual_size)

/*
 * zoidfs_null
 * This function implements a noop operation. The IOD returns a 1-byte message
 * to the CN.
 *
 */
int zoidfs_null(void) {
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_NULL);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /* calculate the size of the recv buffer */
    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status);

    /* calculate the size of the send buffer */
    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id);

    /* allocate the bmi send buffer */
    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_null: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto null_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);

    /* encode the op id */
    if((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK)
    {
        goto null_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg); 

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_null: BMI_memalloc() failed.\n");
        ret = ZFSERR_XDR;
        goto null_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK)
    {
        goto null_cleanup;
    }

null_cleanup:

    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle from
 * the ION.
 */
int zoidfs_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr) {
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_GET_ATTR);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /* calculate the send buffer size */
    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle) +
                          zoidfs_xdr_size_processor(ZFS_ATTR_T, attr);

    /* calculate the recv buffer size */
    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_ATTR_T, attr);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_getattr: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto getattr_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) 
    {
        goto getattr_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)handle, &send_msg.send_xdr)) != ZFS_OK) {
        goto getattr_cleanup;
    }
    if((ret = zoidfs_xdr_processor(ZFS_ATTR_T, attr, &send_msg.send_xdr)) != ZFS_OK) {
        goto getattr_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_getattr: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto getattr_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        goto getattr_cleanup;
    }

    /* if the op status is not ZFS_OK, do not decode any more and exit */
    if(recv_msg.op_status == ZFS_OK)
    {
        if ((ret = zoidfs_xdr_processor(ZFS_ATTR_T, attr, &recv_msg.recv_xdr)) != ZFS_OK) {
            goto getattr_cleanup;
        }
    }

getattr_cleanup:

    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg); 

    return ret;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
int zoidfs_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t *attr) {

    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_SET_ATTR);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_ATTR_T, attr);
    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle) +
                          zoidfs_xdr_size_processor(ZFS_SATTR_T, (void *)sattr) +
                          zoidfs_xdr_size_processor(ZFS_ATTR_T, attr);

   ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_setattr: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto setattr_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        goto setattr_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)handle, &send_msg.send_xdr)) != ZFS_OK) {
        goto setattr_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_SATTR_T, (zoidfs_sattr_t *)sattr, &send_msg.send_xdr)) != ZFS_OK) {
        goto setattr_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_ATTR_T, attr, &send_msg.send_xdr)) != ZFS_OK) {
        goto setattr_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        ret = ZFSERR_MISC;
        goto setattr_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        goto setattr_cleanup;
    }
   
    /* if operation failed, return and do not try to decode the xdr data */ 
    if(recv_msg.op_status == ZFS_OK)
    {
        if (attr) {
            if ((ret = zoidfs_xdr_processor(ZFS_ATTR_T, attr, &recv_msg.recv_xdr) != ZFS_OK)) {
                goto setattr_cleanup;
            }
        }
    }

setattr_cleanup:
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);
    return ret;
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 */
int zoidfs_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length) {
    int ret = ZFS_OK;
    uint64_t buffer_length_uint64_t = (uint64_t)buffer_length;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* internal buffers */
    char * intl_buffer = NULL;
    uint64_t intl_buffer_length_uint64_t = zfsmin(buffer_length_uint64_t + 1, ZOIDFS_PATH_MAX + 1);

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_READLINK);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /* setup internal buffers... ZOIDFS_PATH_MAX + 1 */
    intl_buffer = (char *) malloc(sizeof(char) * intl_buffer_length_uint64_t); 

    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_T, &buffer_length_uint64_t);

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          XDRSIZE_BUFFER_T(intl_buffer_length_uint64_t);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_readlink: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto readlink_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readlink_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)handle, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readlink_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT64_T, &intl_buffer_length_uint64_t, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readlink_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_readlink: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto readlink_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto readlink_cleanup;
    }
    if (recv_msg.op_status == ZFS_OK)
    {
       zoidfs_buffer_transfer_t rl_buffer_transfer;
       rl_buffer_transfer.data = intl_buffer;
       rl_buffer_transfer.len = intl_buffer_length_uint64_t;
       if ((ret = zoidfs_xdr_processor(ZFS_BUFFER_T, &rl_buffer_transfer, &recv_msg.recv_xdr)) != ZFS_OK) {
          
          goto readlink_cleanup;
       }

        /* copy the internal buffer to the user buffer */
        memcpy(buffer, intl_buffer, intl_buffer_length_uint64_t - 1);
    }

readlink_cleanup:

    if(intl_buffer)
    {
        free(intl_buffer);
        intl_buffer = NULL;
    }

    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_lookup
 * This function returns the file handle associated with the given file or
 * directory name.
 */
int zoidfs_lookup(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_handle_t *handle) {
    zoidfs_null_param_t null_param;
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_LOOKUP);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_lookup: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto lookup_cleanup;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) + 
                              XDRSIZE_CSTRING_PATH_T(full_path);
    } else {
        null_param = 0;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) +
                              zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)parent_handle) +
                              XDRSIZE_CSTRING_NAME_T(component_name);
    }
    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, handle);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_lookup: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto lookup_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto lookup_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto lookup_cleanup;
    }

    if (full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto lookup_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto lookup_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto lookup_cleanup;
        }
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_lookup: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto lookup_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto lookup_cleanup;
    }

    if(recv_msg.op_status == ZFS_OK)
    {
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, handle, &recv_msg.recv_xdr)) != ZFS_OK) {
            
            goto lookup_cleanup;
        }
    }

lookup_cleanup:
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;

}


/*
 * zoidfs_remove
 * This function removes the given file or directory.
 */
int zoidfs_remove(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_cache_hint_t *parent_hint) {
    int ret = ZFS_OK;
    zoidfs_cache_hint_t hint;
    zoidfs_null_param_t null_param;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_REMOVE);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_remove: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto remove_cleanup;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) +
                              XDRSIZE_CSTRING_PATH_T(full_path);
    } else {
        null_param = 0;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) +
                              zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)parent_handle) +
                              XDRSIZE_CSTRING_NAME_T(component_name);
    }

    if (parent_hint) {
        recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                              zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, parent_hint);
    } else {
        recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                              zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint);
    }

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_remove: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto remove_cleanup; 
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto remove_cleanup; 
    }
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto remove_cleanup; 
    }

    if (full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto remove_cleanup; 
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto remove_cleanup; 
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto remove_cleanup; 
        }
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_remove: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto remove_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto remove_cleanup;
    }
    if(recv_msg.op_status == ZFS_OK)
    {
        if (parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto remove_cleanup;
            }
        }
    }

remove_cleanup:
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
int zoidfs_commit(const zoidfs_handle_t *handle) {
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_COMMIT);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status);
    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_commit: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto commit_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto commit_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)handle, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto commit_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_commit: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto commit_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto commit_cleanup;
    }

commit_cleanup:
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_create
 * This function creates a new file.
 */
int zoidfs_create(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
                  int *created) {
    zoidfs_null_param_t null_param;
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_CREATE);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_create: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto create_cleanup;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) +
                              XDRSIZE_CSTRING_PATH_T(full_path) +
                              zoidfs_xdr_size_processor(ZFS_SATTR_T, (void *)sattr);
    } else {
        null_param = 0;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) +
                              zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)parent_handle) +
                              XDRSIZE_CSTRING_NAME_T(component_name) +
                              zoidfs_xdr_size_processor(ZFS_SATTR_T, (void *)sattr);
    }

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle) +
                          zoidfs_xdr_size_processor(ZFS_INT_T, created);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_create: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto create_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto create_cleanup;
    }
    if ((zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto create_cleanup;
    }

    if (full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto create_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto create_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto create_cleanup;
        }
    }

    if ((zoidfs_xdr_processor(ZFS_SATTR_T, (void *)sattr, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto create_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_create: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto create_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg); 

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto create_cleanup;
    }

    if(recv_msg.op_status == ZFS_OK)
    {
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, handle, &recv_msg.recv_xdr)) != ZFS_OK) {
            
            goto create_cleanup;
        }

        /* Decode the "created" field only if the caller wants it */
        if (created) {
            if ((ret = zoidfs_xdr_processor(ZFS_INT_T, created, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto create_cleanup;
            }
        }
    }

create_cleanup:
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}

/*
 * zoidfs_rename
 * This function renames an existing file or directory.
 */
int zoidfs_rename(const zoidfs_handle_t *from_parent_handle,
                  const char *from_component_name,
                  const char *from_full_path,
                  const zoidfs_handle_t *to_parent_handle,
                  const char *to_component_name,
                  const char *to_full_path,
                  zoidfs_cache_hint_t *from_parent_hint,
                  zoidfs_cache_hint_t *to_parent_hint) {
    int ret = ZFS_OK;
    zoidfs_cache_hint_t hint;
    zoidfs_null_param_t from_null_param, to_null_param;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_RENAME);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_rename: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto rename_cleanup;
    }
    if ((!to_parent_handle || !to_component_name) && !to_full_path) {
        fprintf(stderr, "zoidfs_rename: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto rename_cleanup;
    }

    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id);
    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (from_full_path) {
        from_null_param = 1;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &from_null_param) +
                               XDRSIZE_CSTRING_PATH_T(from_full_path);
    } else {
        from_null_param = 0;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &from_null_param) +
                               zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)from_parent_handle) +
                               XDRSIZE_CSTRING_NAME_T(from_component_name);
    }
    if (to_full_path) {
        to_null_param = 1;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &to_null_param) +
                               XDRSIZE_CSTRING_PATH_T(to_full_path);
    } else {
        to_null_param = 0;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &to_null_param) +
                               zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)to_parent_handle) +
                               XDRSIZE_CSTRING_NAME_T(to_component_name);
    }

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint) +
                          zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_rename: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto rename_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);

    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto rename_cleanup;
    }

    /*
     * encode the from file data.
     */
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &from_null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto rename_cleanup;
    }
    if (from_full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)from_full_path;
        fp_transfer.len =  ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto rename_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)from_component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)from_parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto rename_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK)
        {
            
            goto rename_cleanup;
        }
    }

    /*
     * encode the to file data.
     */
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &to_null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto rename_cleanup;
    }
    if (to_full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)to_full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto rename_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)to_component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)to_parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto rename_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK)
        {
            
            goto rename_cleanup;
        }
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_rename: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto rename_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto rename_cleanup;
    }

    if(recv_msg.op_status == ZFS_OK)
    {
        if (from_parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, from_parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                goto rename_cleanup;
            }
        } else {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                goto rename_cleanup;
            }
        }
        if (to_parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, to_parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                goto rename_cleanup;
            }
        } else {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto rename_cleanup;
            }
        }
    }

rename_cleanup:
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_link
 * This function creates a hard link.
 */
int zoidfs_link(const zoidfs_handle_t *from_parent_handle,
                const char *from_component_name,
                const char *from_full_path,
                const zoidfs_handle_t *to_parent_handle,
                const char *to_component_name,
                const char *to_full_path,
                zoidfs_cache_hint_t *from_parent_hint,
                zoidfs_cache_hint_t *to_parent_hint) {
    zoidfs_cache_hint_t hint;
    zoidfs_null_param_t from_null_param, to_null_param;
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_LINK);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_link: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto link_cleanup;
    }
    if ((!to_parent_handle || !to_component_name) && !to_full_path) {
        fprintf(stderr, "zoidfs_link: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto link_cleanup;
    }

    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id);
    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (from_full_path) {
        from_null_param = 1;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &from_null_param) +
                               XDRSIZE_CSTRING_PATH_T(from_full_path); 
    } else {
        from_null_param = 0;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &from_null_param) +
                               zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)from_parent_handle) +
                               XDRSIZE_CSTRING_NAME_T(from_component_name);
    }
    if (to_full_path) {
        to_null_param = 1;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &to_null_param) +
                               XDRSIZE_CSTRING_PATH_T(to_full_path);
    } else {
        to_null_param = 0;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &to_null_param) +
                               zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)to_parent_handle) +
                               XDRSIZE_CSTRING_NAME_T(to_component_name);
    }

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint) +
                          zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_link: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto link_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T,&send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        fprintf(stderr, "zoidfs_link: xdr_zoidfs_op_id_t() failed.\n");
        
        goto link_cleanup;
    }

    /* 
     * encode null param with the from handle or path
     */
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &from_null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto link_cleanup;
    }
    if (from_full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)from_full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto link_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)from_component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)from_parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto link_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto link_cleanup;
        }
    }

    /* 
     * encode null param with the to handle or path
     */
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &to_null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto link_cleanup;
    }
    if (to_full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)to_full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto link_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)to_component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)to_parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto link_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto link_cleanup;
        }
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_link: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto link_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto link_cleanup;
    }

    if (recv_msg.op_status == ZFS_OK)
    {
        if (from_parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, from_parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto link_cleanup;
            }
        } else {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto link_cleanup;
            }
        }
        if (to_parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, to_parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto link_cleanup;
            }
        } else {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto link_cleanup;
            }
        }

    }

link_cleanup:
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_symlink
 * This function creates a symbolic link.
 */
int zoidfs_symlink(const zoidfs_handle_t *from_parent_handle,
                   const char *from_component_name,
                   const char *from_full_path,
                   const zoidfs_handle_t *to_parent_handle,
                   const char *to_component_name,
                   const char *to_full_path,
                   const zoidfs_sattr_t *sattr,
                   zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs_cache_hint_t *to_parent_hint) {
    zoidfs_cache_hint_t hint;
    zoidfs_null_param_t from_null_param, to_null_param;
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_SYMLINK);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_symlink: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto symlink_cleanup;
    }
    if ((!to_parent_handle || !to_component_name) && !to_full_path) {
        fprintf(stderr, "zoidfs_symlink: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto symlink_cleanup;
    }

    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id);
    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (from_full_path) 
    {
        from_null_param = 1;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &from_null_param) +
                               XDRSIZE_CSTRING_PATH_T(from_full_path);
    } else {
        from_null_param = 0;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &from_null_param) +
                               zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)from_parent_handle) +
                               XDRSIZE_CSTRING_NAME_T(from_component_name);
    }
    if (to_full_path) {
        to_null_param = 1;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &to_null_param) +
                               XDRSIZE_CSTRING_PATH_T(to_full_path);
    } else {
        to_null_param = 0;
        send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &to_null_param) +
                               zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)to_parent_handle) +
                               XDRSIZE_CSTRING_NAME_T(to_component_name);
    }

    send_msg.sendbuflen += zoidfs_xdr_size_processor(ZFS_SATTR_T, (void *)sattr);
    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint) + 
                          zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_symlink: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto symlink_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto symlink_cleanup;
    }

    /*
     * encode the from file data including the null_param and the handle / path / name
     */
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &from_null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto symlink_cleanup;
    }
    if (from_full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)from_full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            goto symlink_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)from_component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)from_parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            goto symlink_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            goto symlink_cleanup;
        }
    }

    /*
     * encode the to file data including the null_param and the handle / path / name
     */
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &to_null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto symlink_cleanup;
    }
    if (to_full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)to_full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            goto symlink_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)to_component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)to_parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            goto symlink_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            goto symlink_cleanup;
        }
    }
    if ((ret = zoidfs_xdr_processor(ZFS_SATTR_T, (void *)sattr, &send_msg.send_xdr)) != ZFS_OK) {
        goto symlink_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_symlink: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto symlink_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto symlink_cleanup;
    }

    if (recv_msg.op_status == ZFS_OK)
    {
        if (from_parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, from_parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto symlink_cleanup;
            }
        } else {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto symlink_cleanup;
            }
        }
        if (to_parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, to_parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto symlink_cleanup;
            }
        } else {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto symlink_cleanup;
            }
        }
    }

symlink_cleanup:
    
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}

/*
 * zoidfs_mkdir
 * This function creates a new directory.
 */
int zoidfs_mkdir(const zoidfs_handle_t *parent_handle,
                 const char *component_name, const char *full_path,
                 const zoidfs_sattr_t *sattr,
                 zoidfs_cache_hint_t *parent_hint) {
    zoidfs_cache_hint_t hint;
    zoidfs_null_param_t null_param;
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_MKDIR);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_mkdir: Invalid path parameters.\n");
        ret = ZFSERR_MISC;
        goto mkdir_cleanup;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) +
                              XDRSIZE_CSTRING_PATH_T(full_path) +
                              zoidfs_xdr_size_processor(ZFS_SATTR_T, (void *)sattr);
    } else {
        null_param = 0;
        send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                              zoidfs_xdr_size_processor(ZFS_NULL_PARAM_T, &null_param) +
                              zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)parent_handle) +
                              XDRSIZE_CSTRING_NAME_T(component_name) +
                              zoidfs_xdr_size_processor(ZFS_SATTR_T, (void *)sattr);
    }

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                 zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_mkdir: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto mkdir_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto mkdir_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_NULL_PARAM_T, &null_param, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto mkdir_cleanup;
    }

    if (full_path) {
        zoidfs_path_transfer_t fp_transfer;
        fp_transfer.data = (void *)full_path;
        fp_transfer.len = ZOIDFS_PATH_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_PATH_T, &fp_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto mkdir_cleanup;
        }
    } else {
        zoidfs_name_transfer_t cn_transfer;
        cn_transfer.data = (void *)component_name;
        cn_transfer.len = ZOIDFS_NAME_MAX + 1;
        if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto mkdir_cleanup;
        }
        if ((ret = zoidfs_xdr_processor(ZFS_CSTRING_NAME_T, &cn_transfer, &send_msg.send_xdr)) != ZFS_OK) {
            
            goto mkdir_cleanup;
        }
    }

    if ((ret = zoidfs_xdr_processor(ZFS_SATTR_T, (void *)sattr, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto mkdir_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_mkdir: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto mkdir_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto mkdir_cleanup;
    }

    if(recv_msg.op_status == ZFS_OK)
    {
        if (parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto mkdir_cleanup;
            }
        }
        else
        {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) {
                
                goto mkdir_cleanup;
            }
        }
    }

mkdir_cleanup:
    
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_readdir
 * This function returns the dirents from the specified parent directory. The
 * cookie is a pointer which specifies where in the directory to start
 * fetching the dirents from.
 */
int zoidfs_readdir(const zoidfs_handle_t *parent_handle,
                   zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                   zoidfs_dirent_t *entries, uint32_t flags,
                   zoidfs_cache_hint_t *parent_hint) {
    uint32_t entry_count = *entry_count_; /* workaround for 32bit */
    zoidfs_cache_hint_t hint;
    dirent_t_transfer trans; 
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_READDIR);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /* server sends int32_t (returncode) followed by int32_t (entry_count)
    * followed by an array of zoidfs_dirent_t followed by the
    * zoidfs_cache_hint_t */

    /* checking there is no overflow... this needs to be improved */
    assert(entry_count == *entry_count_);

    trans.count = &entry_count; 
    trans.entries = &entries; 
    trans.maxcount = entry_count; 

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_UINT32_T, &entry_count) +
                          zoidfs_xdr_size_processor(ZFS_CACHE_HINT_T, &hint) +
                          XDRSIZE_BUFFER_T(xdr_zoidfs_dirent_array_size (entry_count)); 

    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)parent_handle) +
                          zoidfs_xdr_size_processor(ZFS_DIRENT_COOKIE_T, (void *)&cookie) +
                          zoidfs_xdr_size_processor(ZFS_UINT32_T, &entry_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT32_T, &flags);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_readdir: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto readdir_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readdir_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)parent_handle, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readdir_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_DIRENT_COOKIE_T, &cookie, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readdir_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT32_T, &entry_count, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readdir_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT32_T, &flags, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto readdir_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_readdir: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto readdir_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto readdir_cleanup;
    }

    /* If an error occurs, no dir entries are returned */
    if (recv_msg.op_status == ZFS_OK)
    {
       if ((ret = zoidfs_xdr_processor(ZFS_UINT32_T, &entry_count, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto readdir_cleanup;
       }

       if ((ret = zoidfs_xdr_processor(ZFS_DIRENT_TRANSFER_T, &trans, &recv_msg.recv_xdr)) != ZFS_OK)
       {
          
          goto readdir_cleanup;
       }
       if (parent_hint) {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, parent_hint, &recv_msg.recv_xdr)) != ZFS_OK) 
            {
                
                goto readdir_cleanup;
            }
        }
        else
        {
            if ((ret = zoidfs_xdr_processor(ZFS_CACHE_HINT_T, &hint, &recv_msg.recv_xdr)) != ZFS_OK) 
            {
                
                goto readdir_cleanup;
            }
        }
    }

    /* set the number of entries found by readdir() */
    *entry_count_ = entry_count;

readdir_cleanup:
    
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}


/*
 * zoidfs_resize
 * This function resizes the file associated with the file handle.
 */
int zoidfs_resize(const zoidfs_handle_t *handle, uint64_t size) {
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_RESIZE);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status);
    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_T, &size);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_resize: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto resize_cleanup;
    }

    /* Encode the function parameters using XDR */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto resize_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)handle, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto resize_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT64_T, &size, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto resize_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_resize: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto resize_cleanup;
    }

    /* Do a BMI receive in recvbuf */
    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto resize_cleanup;
    }

resize_cleanup:
   
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}

/*
 * zoidfs_write
 * This function implements the zoidfs write call.
 */
int zoidfs_write(const zoidfs_handle_t *handle, size_t mem_count_,
                 const void *mem_starts[], const size_t mem_sizes_[],
                 size_t file_count_, const uint64_t file_starts[],
                 uint64_t file_sizes[]) {
    size_t i;
    uint32_t mem_count = mem_count_;
    bmi_size_t * mem_sizes = (bmi_size_t*)malloc(sizeof(bmi_size_t) * mem_count);
    uint32_t file_count = file_count_;
    uint64_t pipeline_size = 0;
    size_t total_size = 0;
    zoidfs_uint64_array_transfer_t mem_sizes_transfer;
    zoidfs_uint64_array_transfer_t file_starts_transfer;
    zoidfs_uint64_array_transfer_t file_sizes_transfer;
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_WRITE);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /* init the transfer array wrappers */ 
    mem_sizes_transfer.data = (void *)mem_sizes;
    mem_sizes_transfer.len = mem_count;
    file_starts_transfer.data = (void *)file_starts;
    file_starts_transfer.len = file_count;
    file_sizes_transfer.data = (void *)file_sizes;
    file_sizes_transfer.len = file_count;

    /* if file_count or mem_count are 0, return imeadietly */
    if(file_count == 0 || mem_count == 0)
    {
        ret = ZFS_OK;
        goto write_cleanup;
    }

#define PIPELINE_SIZE (1024UL * 1024 * 8)
    for (i = 0; i < mem_count_; i++) {
        total_size += mem_sizes_[i];
        mem_sizes[i] = (bmi_size_t) mem_sizes_[i];
        if (mem_sizes_[i] > ZOIDFS_BUFFER_MAX)
            pipeline_size = PIPELINE_SIZE;
    }
    if (total_size >= PIPELINE_SIZE)
        pipeline_size = PIPELINE_SIZE;

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &file_count);
    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle) +
                          zoidfs_xdr_size_processor(ZFS_UINT32_T, &mem_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &mem_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT32_T, &file_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &file_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &file_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_T, &pipeline_size);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_write: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto write_cleanup;
    }

    /*
     * Encode the function parameters using XDR. We will NOT encode
     * the data.
     */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto write_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)handle, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto write_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT32_T, &mem_count, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto write_cleanup;
    }
    if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &mem_sizes_transfer, &send_msg.send_xdr)) != ZFS_OK)
    {
        
        goto write_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT32_T, &file_count, &send_msg.send_xdr)) != ZFS_OK)
    {
        
        goto write_cleanup;
    }
    if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &file_starts_transfer, &send_msg.send_xdr)) != ZFS_OK)
    {  
        
        goto write_cleanup;
    }
    if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &file_sizes_transfer, &send_msg.send_xdr)) != ZFS_OK)
    {
        
        goto write_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT64_T, &pipeline_size, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto write_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Send the data using an expected BMI message */
    if (pipeline_size == 0) {
        /* No Pipelining */
        if (mem_count == 1) {
            /* Contiguous write */
            ret = bmi_comm_send(peer_addr, mem_starts[0], mem_sizes[0],
                                send_msg.tag, context);
        } else {
            /* Strided writes */
            ret = bmi_comm_send_list(peer_addr,
                                     mem_count, mem_starts, mem_sizes, send_msg.tag,
                                     context);
        }
    } else {
        /* Pipelining */
        ret = zoidfs_write_pipeline(peer_addr, pipeline_size,
                                    mem_count, mem_starts, mem_sizes,
                                    send_msg.tag, context);
    }


    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_write: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto write_cleanup;
    }

    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto write_cleanup;
    }
    if(recv_msg.op_status == ZFS_OK)
    {
        file_sizes_transfer.data = (void *)file_sizes;
        file_sizes_transfer.len = file_count;
        if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &file_sizes_transfer, &recv_msg.recv_xdr)) != ZFS_OK)
        {
            
            goto write_cleanup;
        }
    }

write_cleanup:
    
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    if(mem_sizes)
    {
        free(mem_sizes);
    }
    return ret;
}

static int zoidfs_write_pipeline(BMI_addr_t peer_addr, uint64_t pipeline_size,
                                 size_t list_count, const void ** buf_list,
                                 const bmi_size_t size_list[], bmi_msg_tag_t tag,
                                 bmi_context_id context) {
    int np = 0;
    int ret = ZFS_OK;
    size_t i;
    uint64_t st = 0;
    uint64_t st_mem = 0;

    /* TODO: st_memofs probably shouldn't be 64 bit but whatever the platform
     * size_t is */
    uint64_t st_memofs = 0;
    uint64_t total_size = 0;

    for (i = 0; i < list_count; i++)
        total_size += size_list[i];

    while (st < total_size) {
        uint64_t en = 0;
        uint64_t en_mem = 0;
        uint64_t en_memofs = 0;
        size_t p_list_count = 0;
        const char ** p_buf_list = NULL;
        bmi_size_t * p_size_list = NULL;
        for (i = 0; i < list_count; i++) {
            if (st + pipeline_size <= en + size_list[i]) {
                en_mem = i;
                en_memofs = st + pipeline_size - en;
                en += en_memofs;
                break;
            }
            en += size_list[i];
            if (i == list_count -1) {
                en_mem = i;
                en_memofs = size_list[i];
            }
        }
        /* create next request */
        p_list_count = en_mem + 1 - st_mem;
        p_buf_list = (const char**)malloc(sizeof(char*) * p_list_count);
        p_size_list = (bmi_size_t*)malloc(sizeof(size_t) * p_list_count);
        if (st_mem == en_mem) {
            p_buf_list[0] = ((const char*)buf_list[st_mem]) + st_memofs;
            assert(en_memofs > st_memofs);
            p_size_list[0] = en_memofs - st_memofs;
        } else {
            for (i = st_mem; i <= en_mem; i++) {
                if (i == st_mem) {
                    p_buf_list[i] = ((const char*)buf_list[i]) + st_memofs;
                    p_size_list[i] = size_list[i] - st_memofs;
                } else if (i == en_mem) {
                    p_buf_list[i] = (const char*)buf_list[i];
                    p_size_list[i] = en_memofs;
                } else {
                    p_buf_list[i] = (const char*)buf_list[i];
                    p_size_list[i] = size_list[i];
                }
                assert(p_size_list[i] > 0);
            }
        }
        /* send it */
        {
            size_t p_total_size = 0;
            for (i = 0; i < p_list_count; i++) p_total_size += p_size_list[i];
            /* assert(p_total_size == min(pipeline_size, total_size -st)); */
            ret = bmi_comm_send_list(peer_addr, p_list_count, (const void**)p_buf_list,
                                     p_size_list, tag, context);
        }
        /* next */
        st = en;
        st_mem = en_mem;
        st_memofs = en_memofs;
        if (st_mem < (uint64_t) list_count && (uint64_t) size_list[st_mem] == st_memofs) {
            st_mem++;
            st_memofs = 0;
        }
        free(p_buf_list);
        free(p_size_list);
        np++;
    }
    return 0;
}

/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
int zoidfs_read(const zoidfs_handle_t *handle, size_t mem_count_,
                void *mem_starts[], const size_t mem_sizes_[],
                size_t file_count_, const zoidfs_file_ofs_t file_starts[],
                zoidfs_file_size_t file_sizes[]) {
    size_t i;
    uint32_t mem_count = mem_count_;
    bmi_size_t * mem_sizes = (bmi_size_t*)malloc(sizeof(bmi_size_t) * mem_count);
    uint32_t file_count = file_count_;
    uint64_t pipeline_size = 0;
    size_t total_size = 0;
    zoidfs_uint64_array_transfer_t mem_sizes_transfer;
    zoidfs_uint64_array_transfer_t file_starts_transfer;
    zoidfs_uint64_array_transfer_t file_sizes_transfer;
    int ret = ZFS_OK;
    zoidfs_send_msg_t send_msg;
    zoidfs_recv_msg_t recv_msg;

    /* init the zoidfs xdr data */
    ZOIDFS_SEND_MSG_INIT(send_msg, ZOIDFS_PROTO_READ);
    ZOIDFS_RECV_MSG_INIT(recv_msg);

    /* init the transfer array wrappers */
    mem_sizes_transfer.data = (void *)mem_sizes;
    mem_sizes_transfer.len = mem_count;
    file_starts_transfer.data = (void *)file_starts;
    file_starts_transfer.len = file_count;
    file_sizes_transfer.data = (void *)file_sizes;
    file_sizes_transfer.len = file_count;

    /* if file_count or mem_count are 0, return imeadietly */
    if(file_count == 0 || mem_count == 0)
    {
        ret = ZFS_OK;
        goto read_cleanup;
    }

    for (i = 0; i < mem_count_; i++) {
        total_size += mem_sizes_[i];
        mem_sizes[i] = (bmi_size_t)mem_sizes_[i];
        if (mem_sizes_[i] > ZOIDFS_BUFFER_MAX)
            pipeline_size = PIPELINE_SIZE;
    }
    if (total_size >= PIPELINE_SIZE)
        pipeline_size = PIPELINE_SIZE;

    recv_msg.recvbuflen = zoidfs_xdr_size_processor(ZFS_OP_STATUS_T, &recv_msg.op_status) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &file_count);
    send_msg.sendbuflen = zoidfs_xdr_size_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id) +
                          zoidfs_xdr_size_processor(ZFS_HANDLE_T, (void *)handle) +
                          zoidfs_xdr_size_processor(ZFS_UINT32_T, &mem_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &mem_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT32_T, &file_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &file_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_ARRAY_T, &file_count) +
                          zoidfs_xdr_size_processor(ZFS_UINT64_T, &pipeline_size);

    ZOIDFS_SEND_ALLOC_BUFFER(send_msg);
    if (!send_msg.sendbuf) {
        fprintf(stderr, "zoidfs_read: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto read_cleanup;
    }

    /*
     * Encode the function parameters using XDR. We will NOT encode/decode
     * the data.
     */
    ZOIDFS_SEND_XDR_MEMCREATE(send_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_ID_T, &send_msg.zoidfs_op_id, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto read_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_HANDLE_T, (void *)handle, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto read_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT32_T, &mem_count, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto read_cleanup;
    }
    if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &mem_sizes_transfer, &send_msg.send_xdr)) != ZFS_OK)
    {
        
        goto read_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT32_T, &file_count, &send_msg.send_xdr)) != ZFS_OK)
    {
        
        goto read_cleanup;
    }
    if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &file_starts_transfer, &send_msg.send_xdr)) != ZFS_OK)
    {
        
        goto read_cleanup;
    }
    if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &file_sizes_transfer, &send_msg.send_xdr)) != ZFS_OK)
    {
        
        goto read_cleanup;
    }
    if ((ret = zoidfs_xdr_processor(ZFS_UINT64_T, &pipeline_size, &send_msg.send_xdr)) != ZFS_OK) {
        
        goto read_cleanup;
    }

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = ZOIDFS_BMI_COMM_SENDU(send_msg);

    /* Receive the data from the IOD */
    if (pipeline_size == 0) {
        /* No Pipelining */
        if (mem_count == 1) {
            /* Contiguous read */
            ret = bmi_comm_recv(peer_addr, mem_starts[0], ZOIDFS_BUFFER_MAX, recv_msg.tag,
                                context, &recv_msg.actual_size);
        } else {
            /* Strided reads */
            ret = bmi_comm_recv_list(peer_addr, mem_count,
                                     mem_starts, mem_sizes,
                                     recv_msg.tag, context);
        }
    } else {
        /* Pipelining */
        ret = zoidfs_read_pipeline(peer_addr, pipeline_size,
                                   mem_count, mem_starts, mem_sizes,
                                   recv_msg.tag, context);
    }

    /* Wait for the response from the ION */
    ZOIDFS_RECV_ALLOC_BUFFER(recv_msg);
    if (!recv_msg.recvbuf) {
        fprintf(stderr, "zoidfs_read: BMI_memalloc() failed.\n");
        ret = ZFSERR_MISC;
        goto read_cleanup;
    }

    ret = ZOIDFS_BMI_COMM_RECV(recv_msg);

    /* Decode the ION response */
    ZOIDFS_RECV_XDR_MEMCREATE(recv_msg);
    if ((ret = zoidfs_xdr_processor(ZFS_OP_STATUS_T, &recv_msg.op_status, &recv_msg.recv_xdr)) != ZFS_OK) {
        
        goto read_cleanup;
    }
    if(recv_msg.op_status == ZFS_OK)
    {
        file_sizes_transfer.data = (void *)file_sizes;
        file_sizes_transfer.len = file_count;
        if((ret = zoidfs_xdr_processor(ZFS_UINT64_ARRAY_T, &file_sizes_transfer, &recv_msg.recv_xdr)) != ZFS_OK)
        {
            goto read_cleanup;
        }
    }

read_cleanup:

    /* free the temp mem_sizes array */
    if(mem_sizes)
    {
        free(mem_sizes);
    }
        
    if(recv_msg.op_status != ZFS_OK)
    {
        ret = recv_msg.op_status;
    }

    ZOIDFS_RECV_MSG_DESTROY(recv_msg);
    ZOIDFS_SEND_MSG_DESTROY(send_msg);

    return ret;
}

static int zoidfs_read_pipeline(BMI_addr_t peer_addr, uint64_t pipeline_size,
                                size_t list_count, void ** buf_list,
                                const bmi_size_t size_list[], bmi_msg_tag_t tag,
                                bmi_context_id context) {
    int np = 0;
    int ret = ZFS_OK;
    size_t i;
    uint64_t st = 0;
    uint64_t st_mem = 0;
    uint64_t st_memofs = 0;
    uint64_t total_size = 0;
    for (i = 0; i < list_count; i++)
        total_size += size_list[i];
    while (st < total_size) {
        uint64_t en = 0;
        uint64_t en_mem = 0;
        uint64_t en_memofs = 0;
        size_t p_list_count = 0;
        char ** p_buf_list = NULL;
        bmi_size_t * p_size_list = NULL;
        for (i = 0; i < list_count; i++) {
            if (st + pipeline_size <= en + size_list[i]) {
                en_mem = i;
                en_memofs = st + pipeline_size - en;
                en += en_memofs;
                break;
            }
            en += size_list[i];
            if (i == list_count -1) {
                en_mem = i;
                en_memofs = size_list[i];
            }
        }
        /* create next request */
        p_list_count = en_mem + 1 - st_mem;
        p_buf_list = (char**)malloc(sizeof(char*) * p_list_count);
        p_size_list = (bmi_size_t*)malloc(sizeof(size_t) * p_list_count);
        if (st_mem == en_mem) {
            p_buf_list[0] = ((char*)buf_list[st_mem]) + st_memofs;
            assert(en_memofs > st_memofs);
            p_size_list[0] = en_memofs - st_memofs;
        } else {
            for (i = st_mem; i <= en_mem; i++) {
                if (i == st_mem) {
                    p_buf_list[i] = ((char*)buf_list[i]) + st_memofs;
                    p_size_list[i] = size_list[i] - st_memofs;
                } else if (i == en_mem) {
                    p_buf_list[i] = (char*)buf_list[i];
                    p_size_list[i] = en_memofs;
                } else {
                    p_buf_list[i] = (char*)buf_list[i];
                    p_size_list[i] = size_list[i];
                }
                assert(p_size_list[i] > 0);
            }
        }
        /* recv */
        {
            size_t p_total_size = 0;
            for (i = 0; i < p_list_count; i++) p_total_size += p_size_list[i];
            /* assert(p_total_size == std::min(pipeline_size, total_size -st)); */
            ret = bmi_comm_recv_list(peer_addr, p_list_count, (void**)p_buf_list,
                                     p_size_list, tag, context);
        }
        /* next */
        st = en;
        st_mem = en_mem;
        st_memofs = en_memofs;
        if (st_mem < (uint64_t) list_count && (uint64_t) size_list[st_mem] == st_memofs) {
            st_mem++;
            st_memofs = 0;
        }
        free(p_buf_list);
        free(p_size_list);
        np++;
    }
    return 0;
}

/*
 * zoidfs_init
 * Initialize the client subsystems.
 */
int zoidfs_init(void) {
    int ret = ZFS_OK;

#ifdef ZFS_USE_XDR_SIZE_CACHE
    /* get the values for the size cache */
    zoidfs_xdr_size_processor_cache_init();
#endif

    assert(sizeof(size_t) == sizeof(unsigned long));

    /* Initialize BMI */
    ret = BMI_initialize(NULL, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "zoidfs_init: BMI_initialize() failed.\n");
        exit(1);
    }

    /* Create a new BMI context */
    ret = BMI_open_context(&context);
    if (ret < 0) {
        fprintf(stderr, "zoidfs_init: BMI_open_context() failed.\n");
        exit(1);
    }

    /*
     * Pick up the ION hostname from an environment variable (ZOIDFS_ION_NAME).
     */
    ion_name = getenv(ION_ENV);
    if (!ion_name) {
        fprintf(stderr, "zoidfs_init: getenv(\"%s\") failed.\n", ION_ENV);
        exit(1);
    }

    /* Perform an address lookup on the ION */
    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        fprintf(stderr, "zoidfs_init: BMI_addr_lookup() failed, ion_name = %s.\n", ion_name);
        exit(1);
    }

    /* preallocate buffers */
#ifdef ZFS_BMI_FASTMEMALLOC
    zfs_bmi_client_sendbuf = BMI_memalloc(peer_addr, ZFS_BMI_CLIENT_SENDBUF_LEN, BMI_SEND);
    if(!zfs_bmi_client_sendbuf)
    {
        fprintf(stderr, "zoidfs_init: could not allocate send buffer for fast mem alloc.\n");
        exit(1);
    }

    zfs_bmi_client_recvbuf = BMI_memalloc(peer_addr, ZFS_BMI_CLIENT_RECVBUF_LEN, BMI_RECV);
    if(!zfs_bmi_client_recvbuf)
    {
        fprintf(stderr, "zoidfs_init: could not allocate recv buffer for fast mem alloc.\n");
        exit(1);
    }
#endif

    return 0;
}

/*
 * zoidfs_finalize
 * Finalize the client subsystems.
 */
int zoidfs_finalize(void) {
    int ret = ZFS_OK;

    /* cleanup buffers */
#ifdef ZFS_BMI_FASTMEMALLOC
    if(zfs_bmi_client_sendbuf)
    {
        free(zfs_bmi_client_sendbuf);
        zfs_bmi_client_sendbuf = NULL;
    }

    if(zfs_bmi_client_recvbuf)
    {
        free(zfs_bmi_client_recvbuf);
        zfs_bmi_client_recvbuf = NULL;
    }
#endif
    BMI_close_context(context);

    /* Finalize BMI */
    ret = BMI_finalize();
    if (ret < 0) {
        fprintf(stderr, "zoidfs_finalize: BMI_finalize() failed.\n");
        exit(1);
    }
    return 0;
}



/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
