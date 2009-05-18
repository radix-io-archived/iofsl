/*
 * zoidfs_proto.h
 * This file contains definitions specific to the ZOIDFS protocol.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#ifndef _ZOIDFS_PROTO_H_
#define _ZOIDFS_PROTO_H_

#define ZOIDFS_BUFFER_MAX (8 * 1024 * 1024)

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
#define ZOIDFS_PROTO_NULL         0
#define ZOIDFS_PROTO_GET_ATTR     1
#define ZOIDFS_PROTO_SET_ATTR     2
#define ZOIDFS_PROTO_LOOKUP       3
#define ZOIDFS_PROTO_READLINK     4
#define ZOIDFS_PROTO_COMMIT       5
#define ZOIDFS_PROTO_CREATE       6
#define ZOIDFS_PROTO_REMOVE       7
#define ZOIDFS_PROTO_RENAME       8
#define ZOIDFS_PROTO_SYMLINK      9
#define ZOIDFS_PROTO_MKDIR        10
#define ZOIDFS_PROTO_READDIR      11
#define ZOIDFS_PROTO_RESIZE       12
#define ZOIDFS_PROTO_WRITE        13
#define ZOIDFS_PROTO_READ         14
#define ZOIDFS_PROTO_LINK         15


/* ZOIDFS Protocol error codes */
enum {
    ZFSERR_XDR=100,
    ZFSERR_MISC=101
};

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
