#include <assert.h>
/*
 * zoidfs_xdr.c
 * XDR encoding/decoding function implementation for ZOIDFS data structures.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 *
 * Dries Kimpe <dkimpe@mcs.anl.gov> 
 */

#include "iofwd_config.h"
#include "zoidfs_xdr.h"
#include "zoidfs/hints/zoidfs-hints.h"

/* round up to next multiple of 4 */

static inline unsigned int round4 (unsigned int s)
{
   return (((s+3)/4)*4);
}

unsigned int xdr_stringsize (unsigned int maxlen)
{
   return 4 + round4 (maxlen); 
}

/*
 * xdr_zoidfs_op_id_t
 * Encode/decode zoidfs_op_id_t using XDR.
 */
bool_t xdr_zoidfs_op_id_t(XDR *xdrs, zoidfs_op_id_t *op_id) {
    return(xdr_uint32_t(xdrs, op_id));
}


/*
 * xdr_zoidfs_null_param_t
 * Encode/decode zoidfs_null_param_t using XDR.
 */
bool_t xdr_zoidfs_null_param_t(XDR *xdrs, zoidfs_null_param_t *null_param) {
    return(xdr_int32_t(xdrs, null_param));
}

/* encode the zoidfs_file_ofs_t value */
bool_t xdr_zoidfs_file_ofs_t(XDR *xdrs, zoidfs_file_ofs_t * fo) {
    return(xdr_uint64_t(xdrs, fo));
}

/*
 * Encode / decode size_t values
 */
bool_t xdr_size_t(XDR *xdrs, size_t * sizet) {
#if SIZEOF_SIZE_T == 8
    uint64_t _st = *sizet;
    return(xdr_uint64_t(xdrs, &_st));
#else
    uint32_t _st = *sizet;
    return(xdr_uint32_t(xdrs, &_st));
#endif
}

bool_t xdr_string_helper (XDR * xdr, string_transfer * t)
{
   assert (t->buffer); 
   return xdr_string (xdr, &t->buffer, t->maxsize); 
}

/*
 * xdr_zoidfs_op_status_t
 * Encode/decode xdr_zoidfs_op_status_t using XDR.
 */
bool_t xdr_zoidfs_op_status_t(XDR *xdrs, zoidfs_op_status_t *op_status) {
    return(xdr_int32_t(xdrs, op_status));
}


/*
 * xdr_zoidfs_handle_t
 * Encode/decode zoidfs_handle_t using XDR.
 */
bool_t xdr_zoidfs_handle_t(XDR *xdrs, const zoidfs_handle_t *handle) {
    return(xdr_opaque(xdrs, (char *)handle->data, sizeof (handle->data)));
}


/*
 * xdr_zoidfs_dirent_cookie_t
 * Encode/decode zoidfs_dirent_cookie_t using XDR.
 */
bool_t xdr_zoidfs_dirent_cookie_t(XDR *xdrs, zoidfs_dirent_cookie_t *cookie) {
    return(xdr_uint64_t(xdrs, cookie));
}

/* return max size required for entry_count dirent structures */ 
unsigned int xdr_zoidfs_dirent_array_size (unsigned int entry_count)
{
   zoidfs_dirent_t dummy; 

   unsigned int ret = 
       4 /* number of entries in array */ + 
       entry_count * (4  /* length of string */  
          + round4 (ZOIDFS_NAME_MAX) /* string data */ 
          + xdr_sizeof ((xdrproc_t) xdr_zoidfs_handle_t, &dummy.handle)
          + xdr_sizeof ((xdrproc_t) xdr_zoidfs_attr_t, &dummy.attr)
          + xdr_sizeof ((xdrproc_t) xdr_zoidfs_dirent_cookie_t, &dummy.cookie)
        ); 

   return ret; 

}

/*
 * xdr_zoidfs_dirent_t
 * Encode/decode zoidfs_dirent_t using XDR.
 *
 * This is ridiculous: this function is also called for size calculations,
 * so why should it allocate memory in that case???
 */
bool_t xdr_zoidfs_dirent_t(XDR *xdrs, zoidfs_dirent_t *dirent) 
{
    bool_t ret;

    char * ptr = &dirent->name[0]; 

    ret = xdr_string(xdrs, &ptr, sizeof (dirent->name)-1);
    ret = ret &&  xdr_zoidfs_handle_t(xdrs, &dirent->handle);
    ret = ret &&  xdr_zoidfs_attr_t(xdrs, &dirent->attr);
    ret = ret &&  xdr_zoidfs_dirent_cookie_t(xdrs, &dirent->cookie);

    return(ret);
}


/*
 * xdr_zoidfs_attr_type_t
 * Encode/decode zoidfs_attr_type_t using XDR.
 */
bool_t xdr_zoidfs_attr_type_t(XDR *xdrs, zoidfs_attr_type_t *attr_type) {
    return(xdr_u_int(xdrs, attr_type));
}


/*
 * xdr_zoidfs_time_t
 * Encode/decode zoidfs_time_t using XDR.
 */
bool_t xdr_zoidfs_time_t(XDR *xdrs, zoidfs_time_t *ztime) {
    bool_t ret;

    ret = xdr_uint64_t(xdrs, &ztime->seconds) &&
          xdr_uint32_t(xdrs, &ztime->nseconds);

    return(ret);
}


/*
 * xdr_zoidfs_attr_t
 * Encode/decode zoidfs_attr_t using XDR.
 */
bool_t xdr_zoidfs_attr_t(XDR *xdrs, zoidfs_attr_t *attr) {
    bool_t ret;

    ret = xdr_zoidfs_attr_type_t(xdrs, &attr->type) &&
          xdr_uint16_t(xdrs, &attr->mask) &&
          xdr_uint16_t(xdrs, &attr->mode) &&
          xdr_uint32_t(xdrs, &attr->nlink) &&
          xdr_uint32_t(xdrs, &attr->uid) &&
          xdr_uint32_t(xdrs, &attr->gid) &&
          xdr_uint64_t(xdrs, &attr->size) &&
          xdr_uint32_t(xdrs, &attr->blocksize) &&
          xdr_uint32_t(xdrs, &attr->fsid) &&
          xdr_uint64_t(xdrs, &attr->fileid) &&
          xdr_zoidfs_time_t(xdrs, &attr->atime) &&
          xdr_zoidfs_time_t(xdrs, &attr->mtime) &&
          xdr_zoidfs_time_t(xdrs, &attr->ctime);

    return(ret);
}


/*
 * xdr_zoidfs_cache_hint_t
 * Encode/decode zoidfs_cache_hint_t using XDR.
 */
bool_t xdr_zoidfs_cache_hint_t(XDR *xdrs, zoidfs_cache_hint_t *hint) {
    bool_t ret;

    ret = xdr_uint64_t(xdrs, &hint->size) &&
          xdr_zoidfs_time_t(xdrs, &hint->atime) &&
          xdr_zoidfs_time_t(xdrs, &hint->mtime) &&
          xdr_zoidfs_time_t(xdrs, &hint->ctime);

    return(ret);
}

/*
 * zdr_zoidfs_op_hint_t
 * Encode / decode zoidfs_op_hint_t using XDR
 */
bool_t xdr_zoidfs_op_hint_element_t(XDR * xdrs, char * key, char * value, int valuelen) {
    bool_t ret;
    int key_len = strlen(key) + 1;
    
    ret = xdr_int(xdrs, &key_len); 
    ret = ret && xdr_string(xdrs, &key, key_len);
    ret = ret && xdr_int(xdrs, &valuelen); 
    ret = ret && xdr_string(xdrs, &value, valuelen);

    return(ret);
}

bool_t xdr_zoidfs_op_hint_t(XDR *xdrs, zoidfs_op_hint_t * hint) {
    bool_t ret = 1;
    int hint_size = 0;
    int i = 0;
    int valid_hint = 0;

    if(xdrs->x_op == XDR_ENCODE)
    {
        if(hint)
        {
            zoidfs_hint_get_nkeys(*hint, &hint_size);
        }
        else
        {
            hint_size = 0;
        }

        if(hint_size > 0)
        {
            valid_hint = 1;
            ret = xdr_zoidfs_null_param_t(xdrs, &valid_hint);
            ret = ret && xdr_int(xdrs, &hint_size);
            
            for(i = 0 ; i < hint_size ; i++)
            {
                int flag = 0;
                int keylength = 0;
                int valuelength = 0;
                char * key = NULL;
                char * value = NULL;

                /* get the next key */
                zoidfs_hint_get_nthkeylen(*hint, i, &keylength);
                key = zoidfs_hint_get_nthkey_raw(*hint, i); /* use raw to avoid mem alloc */

                /* get the next value */
                zoidfs_hint_get_valuelen(*hint, key, &valuelength, &flag);
                value = zoidfs_hint_get_raw(*hint, key, valuelength, &flag); /* use raw to avoid mem alloc */

                /* make sure hint data is valid */
                if(key != NULL && value != NULL && valuelength != 0)
                {
                    ret = ret && xdr_zoidfs_op_hint_element_t(xdrs, key, value, valuelength);
                }
            }
        }
        else
        {
            valid_hint = 0;
            ret = xdr_zoidfs_null_param_t(xdrs, &valid_hint);
        }
    }
    else if(xdrs->x_op == XDR_DECODE)
    {
        ret = xdr_zoidfs_null_param_t(xdrs, &valid_hint);

        if(valid_hint && hint)
        {
            ret = ret && xdr_int(xdrs, &hint_size);
            for(i = 0 ; i < hint_size ; i++)
            {
                int keylen = 0;
                int valuelen = 0;
                char * key = NULL;
                char * value = NULL;

                /* get the key */
                ret = ret && xdr_int(xdrs, &keylen);
                key = (char *)malloc(sizeof(char) * keylen);
                ret = ret && xdr_string(xdrs, &key, keylen);

                /* get the value */
                ret = ret && xdr_int(xdrs, &valuelen); 
                value = (char *)malloc(sizeof(char) * valuelen);
                ret = ret && xdr_string(xdrs, &value, valuelen);

                /* make sure hint data is valid */
                if(key != NULL && keylen != 0 && value != NULL && valuelen != 0)
                {
                    zoidfs_hint_set_raw(*hint, key, value, valuelen); /* use raw to avoid mem alloc */
                }
            }
        }
    }
    return ret;
}

/*
 * xdr_zoidfs_sattr_t
 * Encode/decode zoidfs_sattr_t using XDR.
 */
bool_t xdr_zoidfs_sattr_t(XDR *xdrs, zoidfs_sattr_t *attr) {
    bool_t ret;

    ret = xdr_uint16_t(xdrs, &attr->mask) &&
          xdr_uint16_t(xdrs, &attr->mode) &&
          xdr_uint32_t(xdrs, &attr->uid) &&
          xdr_uint32_t(xdrs, &attr->gid) &&
          xdr_uint64_t(xdrs, &attr->size) &&
          xdr_zoidfs_time_t(xdrs, &attr->atime) &&
          xdr_zoidfs_time_t(xdrs, &attr->mtime);

    return(ret);
}

bool_t xdr_zoidfs_dirent_array (XDR * xdr, dirent_t_transfer * t)
{
   unsigned int c = *t->count; 

   /* t->entries is a pointer to a pointer to a set of dirent_t structs */ 
   assert (t->entries);
   assert (*t->entries); 
   bool_t ret = xdr_array (xdr, (char **) t->entries, &c, t->maxcount, 
         sizeof (zoidfs_dirent_t), (xdrproc_t) xdr_zoidfs_dirent_t ); 

   *t->count = c;
   return ret; 
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
