/*
 * zoidfs_ops.h
 * This file defines the ZOIDFS operation macros.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#ifndef _ZOIDFS_OPS_H_
#define _ZOIDFS_OPS_H_

#define ZOIDFS_NULL         0
#define ZOIDFS_GET_ATTR     1
#define ZOIDFS_SET_ATTR     2
#define ZOIDFS_LOOKUP       3
#define ZOIDFS_READLINK     4
#define ZOIDFS_COMMIT       5
#define ZOIDFS_CREATE       6
#define ZOIDFS_REMOVE       7
#define ZOIDFS_RENAME       8
#define ZOIDFS_SYMLINK      9
#define ZOIDFS_MKDIR        10
#define ZOIDFS_READDIR      11
#define ZOIDFS_RESIZE       12
#define ZOIDFS_WRITE        13
#define ZOIDFS_READ         14

#endif /* _ZOIDFS_OPS_H_ */

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
