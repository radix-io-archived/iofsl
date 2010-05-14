/*
 * zoidfs_proto.h
 * This file contains definitions specific to the ZOIDFS protocol.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#ifndef ZOIDFS_PROTO_H
#define ZOIDFS_PROTO_H

#include <stdint.h>

#ifdef __cplusplus
namespace zoidfs
{
#endif

enum { ZOIDFS_BUFFER_MAX = (8 * 1024 * 1024) }; 

/* Op id describes the various zoidfs operations (setattr, getattr etc.) */
typedef uint32_t zoidfs_op_id_t;

/*
 * null_param is used by the server to determine whether the client is passing
 * either the full_path or the parent_handle and component_name.
 */
typedef int32_t zoidfs_null_param_t;

/*
 * zoidfs_op_status_t is used by the server to inform the client of the status
 * of the operation.
 */
typedef int32_t zoidfs_op_status_t;

/* Define the ZOIDFS operations */
enum
{
ZOIDFS_PROTO_NULL      = 0,
ZOIDFS_PROTO_GET_ATTR,
ZOIDFS_PROTO_SET_ATTR,
ZOIDFS_PROTO_LOOKUP,
ZOIDFS_PROTO_READLINK,
ZOIDFS_PROTO_COMMIT,
ZOIDFS_PROTO_CREATE,
ZOIDFS_PROTO_REMOVE,
ZOIDFS_PROTO_RENAME,
ZOIDFS_PROTO_SYMLINK,
ZOIDFS_PROTO_MKDIR,
ZOIDFS_PROTO_READDIR,
ZOIDFS_PROTO_RESIZE,
ZOIDFS_PROTO_WRITE,
ZOIDFS_PROTO_READ,
ZOIDFS_PROTO_LINK,

/* First invalid operation id */
ZOIDFS_PROTO_MAX
};

/* ZOIDFS Protocol error codes */
enum {
    ZFSERR_XDR=100,
    ZFSERR_MISC=101
};


#ifdef __cplusplus
        }  /* namespace */
#endif

#endif /* _ZOIDFS_PROTO_H_ */

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
