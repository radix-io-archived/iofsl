/*
 * zoidfs_xdr.h
 * XDR encoding/decoding function declarations for ZOIDFS data structures.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#ifndef _ZOIDFS_XDR_H_
#define _ZOIDFS_XDR_H_

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<rpc/rpc.h>
#include<sys/time.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

#ifndef MAXARRSIZE
#define MAXARRSIZE	4096
#endif

/* XDR encoding/decoding function declarations */
bool_t xdr_zoidfs_attr_t(XDR *, zoidfs_attr_t *);
bool_t xdr_zoidfs_time_t(XDR *, zoidfs_time_t *);
bool_t xdr_zoidfs_sattr_t(XDR *, zoidfs_sattr_t *);
bool_t xdr_zoidfs_op_id_t(XDR *, zoidfs_op_id_t *);
bool_t xdr_zoidfs_op_status_t(XDR *, zoidfs_op_status_t *);
bool_t xdr_zoidfs_handle_t(XDR *, const zoidfs_handle_t *);
bool_t xdr_zoidfs_dirent_t(XDR *, zoidfs_dirent_t *);
bool_t xdr_zoidfs_attr_type_t(XDR *, zoidfs_attr_type_t *);
bool_t xdr_zoidfs_cache_hint_t(XDR *, zoidfs_cache_hint_t *);
bool_t xdr_zoidfs_null_param_t(XDR *, zoidfs_null_param_t *);
bool_t xdr_zoidfs_dirent_cookie_t(XDR *, zoidfs_dirent_cookie_t *);

#endif /* _ZOIDFS_XDR_H_ */

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
