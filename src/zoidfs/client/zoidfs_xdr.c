/*
 * zoidfs_xdr.c
 * XDR encoding/decoding function implementation for ZOIDFS data structures.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#include "zoidfs_xdr.h"

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
    return(xdr_opaque(xdrs, (char *)handle->data, 32));
}


/*
 * xdr_zoidfs_dirent_cookie_t
 * Encode/decode zoidfs_dirent_cookie_t using XDR.
 */
bool_t xdr_zoidfs_dirent_cookie_t(XDR *xdrs, zoidfs_dirent_cookie_t *cookie) {
    return(xdr_uint64_t(xdrs, cookie));
}


/*
 * xdr_zoidfs_dirent_t
 * Encode/decode zoidfs_dirent_t using XDR.
 */
bool_t xdr_zoidfs_dirent_t(XDR *xdrs, zoidfs_dirent_t *dirent) {
    bool_t ret;
    char *str = malloc(ZOIDFS_NAME_MAX+1);
    if (!str) {
        return 0;
    }
    memset(str, 0, ZOIDFS_NAME_MAX+1);

    /*
     * It turns out that XDR can only encode/decode dynamically allocated
     * strings. Hence, we have to jump through a few hoops here. This is
     * worth investigating though.
     */
    if (xdrs->x_op == XDR_ENCODE) {
        strncpy(str, dirent->name, ZOIDFS_NAME_MAX);
        ret = xdr_string(xdrs, (char **)&str, ZOIDFS_NAME_MAX+1) &&
              xdr_zoidfs_handle_t(xdrs, &dirent->handle) &&
              xdr_zoidfs_attr_t(xdrs, &dirent->attr) &&
              xdr_zoidfs_dirent_cookie_t(xdrs, &dirent->cookie);
    } else {
        ret = xdr_string(xdrs, (char **)&str, ZOIDFS_NAME_MAX+1) &&
              xdr_zoidfs_handle_t(xdrs, &dirent->handle) &&
              xdr_zoidfs_attr_t(xdrs, &dirent->attr) &&
              xdr_zoidfs_dirent_cookie_t(xdrs, &dirent->cookie);
        strncpy(dirent->name, str, ZOIDFS_NAME_MAX);
    }

    free(str);
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

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
