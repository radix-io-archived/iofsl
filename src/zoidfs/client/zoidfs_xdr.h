/*
 * zoidfs_xdr.h
 * XDR encoding/decoding function declarations for ZOIDFS data structures.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 * Dries Kimpe <dkimpe@mcs.anl.gov>
 */

#ifndef ZOIDFS_XDR_H
#define ZOIDFS_XDR_H

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<rpc/rpc.h>
#include<sys/time.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/hints/zoidfs-hints.h"
#include "zoidfs/zoidfs-proto.h"


/* Helper struct to bind arguments */ 
typedef struct 
{
   unsigned int maxcount;
   uint32_t * count;
   zoidfs_dirent_t ** entries;
} dirent_t_transfer; 

/* Helper for string */
typedef struct
{
   unsigned int maxsize; 
   char * buffer; 
} string_transfer; 

/* XDR encoding/decoding function declarations */
bool_t xdr_string_helper (XDR *, string_transfer *); 
bool_t xdr_zoidfs_attr_t(XDR *, zoidfs_attr_t *);
bool_t xdr_zoidfs_time_t(XDR *, zoidfs_time_t *);
bool_t xdr_zoidfs_sattr_t(XDR *, zoidfs_sattr_t *);
bool_t xdr_zoidfs_op_id_t(XDR *, zoidfs_op_id_t *);
bool_t xdr_zoidfs_op_status_t(XDR *, zoidfs_op_status_t *);
bool_t xdr_zoidfs_handle_t(XDR *, const zoidfs_handle_t *);
bool_t xdr_zoidfs_dirent_t(XDR *, zoidfs_dirent_t *);
bool_t xdr_zoidfs_attr_type_t(XDR *, zoidfs_attr_type_t *);
bool_t xdr_zoidfs_cache_hint_t(XDR *, zoidfs_cache_hint_t *);
bool_t xdr_zoidfs_op_hint_t(XDR *, zoidfs_op_hint_t *);
bool_t xdr_zoidfs_op_hint_element_t(XDR *, char * key, char * value, int valuelength);
bool_t xdr_zoidfs_null_param_t(XDR *, zoidfs_null_param_t *);
bool_t xdr_size_t(XDR *, size_t *);
bool_t xdr_zoidfs_file_ofs_t(XDR *, zoidfs_file_ofs_t *);
bool_t xdr_zoidfs_dirent_cookie_t(XDR *, zoidfs_dirent_cookie_t *);
bool_t xdr_zoidfs_dirent_array (XDR * xdr, dirent_t_transfer * t); 

unsigned int xdr_zoidfs_dirent_array_size (unsigned int entry_count);

/* maxlen is the maximum number of CHARACTERS (not including any terminating
 * 0) in the string */ 
unsigned int xdr_stringsize (unsigned int maxlen);

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
