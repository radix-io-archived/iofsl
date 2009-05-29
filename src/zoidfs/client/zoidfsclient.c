/*
 * zoidfs.c
 * Client-side implementation of the ZOIDFS API. The CNs communicate with
 * the IONs over BMI.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#include <bmi.h>
#include "zoidfs/zoidfs.h"
#include "bmi_comm.h"
#include "zoidfs_xdr.h"
#include "zoidfs/zoidfs-proto.h"

#include <assert.h>

static char *ion_name;
static BMI_addr_t peer_addr;
static bmi_msg_tag_t tag = 0;
static bmi_context_id context;

/*
 * zoidfs_null
 * This function implements a noop operation. The IOD returns a 1-byte message
 * to the CN.
 */
int zoidfs_null(void) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_NULL;

    recvbuflen = 1;
/*    C_STATIC_ASSERT(sizeof(bmi_size_t) >= sizeof(unsigned long));  */
    sendbuflen = (bmi_size_t) xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_null: BMI_memalloc() failed.\n");
        exit(1);
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, (unsigned int) sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_null: xdr_zoidfs_op_id_t() failed.\n");
        exit(1);
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_null: BMI_memalloc() failed.\n");
        exit(1);
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return ZFS_OK;
}


/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle from
 * the ION.
 */
int zoidfs_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_GET_ATTR;

    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (void *)handle) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_attr_t, attr);
    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_attr_t, attr);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_getattr: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_getattr: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_getattr: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_attr_t(&xdrs, attr)) {
        fprintf(stderr, "zoidfs_getattr: xdr_zoidfs_attr_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_getattr: BMI_memalloc() failed.\n");
        return ZFSERR_XDR;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_getattr: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_attr_t(&xdrs, attr)) {
        fprintf(stderr, "zoidfs_getattr: xdr_zoidfs_attr_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
int zoidfs_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t *attr) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_SET_ATTR;

    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_attr_t, attr);
    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (void *)handle) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, (void *)sattr) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_attr_t, attr);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_setattr: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_setattr: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_setattr: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_sattr_t(&xdrs, (zoidfs_sattr_t *)sattr)) {
        fprintf(stderr, "zoidfs_setattr: xdr_zoidfs_sattr_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_attr_t(&xdrs, attr)) {
        fprintf(stderr, "zoidfs_setattr: xdr_zoidfs_attr_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_setattr: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_setattr: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (attr) {
        if (!xdr_zoidfs_attr_t(&xdrs, attr)) {
            fprintf(stderr, "zoidfs_setattr: xdr_zoidfs_attr_t() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 */
int zoidfs_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_READLINK;

    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (void *)handle) +
                 xdr_sizeof((xdrproc_t)xdr_u_long, &buffer_length);
    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 ZOIDFS_PATH_MAX;

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_readlink: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_readlink: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_readlink: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_long(&xdrs, &buffer_length)) {
        fprintf(stderr, "zoidfs_readlink: xdr_u_long() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_readlink: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_readlink: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_string(&xdrs, (char **)&buffer, ZOIDFS_PATH_MAX)) {
        fprintf(stderr, "zoidfs_readlink: xdr_string() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_lookup
 * This function returns the file handle associated with the given file or
 * directory name.
 */
int zoidfs_lookup(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_handle_t *handle) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_LOOKUP;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_lookup: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     ZOIDFS_PATH_MAX;
    } else {
        null_param = 0;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                (void *)parent_handle) +
                     ZOIDFS_NAME_MAX;
    }
    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, handle);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_lookup: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_lookup: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        fprintf(stderr, "zoidfs_lookup: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_lookup: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            fprintf(stderr, "zoidfs_lookup: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_lookup: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_lookup: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_lookup: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_lookup: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_remove
 * This function removes the given file or directory.
 */
int zoidfs_remove(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_cache_hint_t *parent_hint) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_cache_hint_t hint;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_REMOVE;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_remove: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     ZOIDFS_PATH_MAX;
    } else {
        null_param = 0;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                (void *)parent_handle) +
                     ZOIDFS_NAME_MAX;
    }

    if (parent_hint) {
        recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t,
                                 &op_status) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t,
                                parent_hint);
    } else {
        recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t,
                                 &op_status) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, &hint);
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_remove: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_remove: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        fprintf(stderr, "zoidfs_remove: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_remove: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            fprintf(stderr, "zoidfs_remove: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_remove: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_remove: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_remove: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, parent_hint)) {
            fprintf(stderr,
                    "zoidfs_remove: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
int zoidfs_commit(const zoidfs_handle_t *handle) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_COMMIT;

    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status);
    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (void *)handle);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_commit: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_commit: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_commit: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_commit: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_commit: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_create
 * This function creates a new file.
 */
int zoidfs_create(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
                  int *created) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_CREATE;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_create: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     ZOIDFS_PATH_MAX +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, (void *)sattr);
    } else {
        null_param = 0;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                (void *)parent_handle) +
                     ZOIDFS_NAME_MAX +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, (void *)sattr);
    }

    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, handle) +
                 xdr_sizeof((xdrproc_t)xdr_int, created);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_create: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_create: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        fprintf(stderr, "zoidfs_create: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_create: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            fprintf(stderr, "zoidfs_create: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_create: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (!xdr_zoidfs_sattr_t(&xdrs, (zoidfs_sattr_t *)sattr)) {
        fprintf(stderr, "zoidfs_create: xdr_zoidfs_sattr_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_create: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_create: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_create: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }

    /* Decode the "created" field only if the caller wants it */
    if (created) {
        if (!xdr_int(&xdrs, created)) {
            fprintf(stderr, "zoidfs_create: xdr_int() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
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
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_cache_hint_t hint;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_RENAME;
    zoidfs_null_param_t from_null_param, to_null_param;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_rename: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }
    if ((!to_parent_handle || !to_component_name) && !to_full_path) {
        fprintf(stderr, "zoidfs_rename: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id);
    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (from_full_path) {
        from_null_param = 1;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &from_null_param) +
                      ZOIDFS_PATH_MAX;
    } else {
        from_null_param = 0;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &from_null_param) +
                      xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                 (void *)from_parent_handle) +
                      ZOIDFS_NAME_MAX;
    }
    if (to_full_path) {
        to_null_param = 1;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &to_null_param) +
                      ZOIDFS_PATH_MAX;
    } else {
        to_null_param = 0;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &to_null_param) +
                      xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                 (void *)to_parent_handle) +
                      ZOIDFS_NAME_MAX;
    }

    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 2 * xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, &hint);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_rename: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_rename: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &from_null_param)) {
        fprintf(stderr, "zoidfs_rename: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &to_null_param)) {
        fprintf(stderr, "zoidfs_rename: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (from_full_path) {
        if (!xdr_string(&xdrs, (char **)&from_full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_rename: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, from_parent_handle)) {
            fprintf(stderr, "zoidfs_rename: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&from_component_name,
                        ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_rename: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (to_full_path) {
        if (!xdr_string(&xdrs, (char **)&to_full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_rename: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, to_parent_handle)) {
            fprintf(stderr, "zoidfs_rename: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&to_component_name, ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_rename: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_rename: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_rename: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (from_parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, from_parent_hint)) {
            fprintf(stderr,
                    "zoidfs_rename: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, &hint)) {
            fprintf(stderr,
                    "zoidfs_rename: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }
    if (to_parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, to_parent_hint)) {
            fprintf(stderr,
                    "zoidfs_rename: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }

    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
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
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_cache_hint_t hint;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_LINK;
    zoidfs_null_param_t from_null_param, to_null_param;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_link: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }
    if ((!to_parent_handle || !to_component_name) && !to_full_path) {
        fprintf(stderr, "zoidfs_link: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id);
    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (from_full_path) {
        from_null_param = 1;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &from_null_param) +
                      ZOIDFS_PATH_MAX;
    } else {
        from_null_param = 0;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &from_null_param) +
                      xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                 (void *)from_parent_handle) +
                      ZOIDFS_NAME_MAX;
    }
    if (to_full_path) {
        to_null_param = 1;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &to_null_param) +
                      ZOIDFS_PATH_MAX;
    } else {
        to_null_param = 0;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &to_null_param) +
                      xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                 (void *)to_parent_handle) +
                      ZOIDFS_NAME_MAX;
    }

    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 2 * xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, &hint);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_link: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_link: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &from_null_param)) {
        fprintf(stderr, "zoidfs_link: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &to_null_param)) {
        fprintf(stderr, "zoidfs_link: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (from_full_path) {
        if (!xdr_string(&xdrs, (char **)&from_full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_link: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, from_parent_handle)) {
            fprintf(stderr, "zoidfs_link: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&from_component_name,
                        ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_link: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (to_full_path) {
        if (!xdr_string(&xdrs, (char **)&to_full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_link: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, to_parent_handle)) {
            fprintf(stderr, "zoidfs_link: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&to_component_name, ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_link: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_link: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_link: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (from_parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, from_parent_hint)) {
            fprintf(stderr,
                    "zoidfs_link: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, &hint)) {
            fprintf(stderr,
                    "zoidfs_link: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }
    if (to_parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, to_parent_hint)) {
            fprintf(stderr,
                    "zoidfs_link: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }

    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
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
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_cache_hint_t hint;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_SYMLINK;
    zoidfs_null_param_t from_null_param, to_null_param;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!from_parent_handle || !from_component_name) && !from_full_path) {
        fprintf(stderr, "zoidfs_symlink: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }
    if ((!to_parent_handle || !to_component_name) && !to_full_path) {
        fprintf(stderr, "zoidfs_symlink: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id);
    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (from_full_path) {
        from_null_param = 1;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &from_null_param) +
                      ZOIDFS_PATH_MAX;
    } else {
        from_null_param = 0;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &from_null_param) +
                      xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                 (void *)from_parent_handle) +
                      ZOIDFS_NAME_MAX;
    }
    if (to_full_path) {
        to_null_param = 1;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &to_null_param) +
                      ZOIDFS_PATH_MAX;
    } else {
        to_null_param = 0;
        sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                 &to_null_param) +
                      xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                 (void *)to_parent_handle) +
                      ZOIDFS_NAME_MAX;
    }

    sendbuflen += xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, (void *)sattr);
    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 2 * xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, &hint);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_symlink: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_symlink: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &from_null_param)) {
        fprintf(stderr, "zoidfs_symlink: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &to_null_param)) {
        fprintf(stderr, "zoidfs_symlink: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (from_full_path) {
        if (!xdr_string(&xdrs, (char **)&from_full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_symlink: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, from_parent_handle)) {
            fprintf(stderr, "zoidfs_symlink: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&from_component_name,
                        ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_symlink: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (to_full_path) {
        if (!xdr_string(&xdrs, (char **)&to_full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_symlink: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, to_parent_handle)) {
            fprintf(stderr, "zoidfs_symlink: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&to_component_name, ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_symlink: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (!xdr_zoidfs_sattr_t(&xdrs, (zoidfs_sattr_t *)sattr)) {
        fprintf(stderr, "zoidfs_symlink: xdr_zoidfs_sattr_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_symlink: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_symlink: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (from_parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, from_parent_hint)) {
            fprintf(stderr,
                    "zoidfs_symlink: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, &hint)) {
            fprintf(stderr,
                    "zoidfs_symlink: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }
    if (to_parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, to_parent_hint)) {
            fprintf(stderr,
                    "zoidfs_symlink: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }

    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_mkdir
 * This function creates a new directory.
 */
int zoidfs_mkdir(const zoidfs_handle_t *parent_handle,
                 const char *component_name, const char *full_path,
                 const zoidfs_sattr_t *sattr,
                 zoidfs_cache_hint_t *parent_hint) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_cache_hint_t hint;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_MKDIR;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        fprintf(stderr, "zoidfs_mkdir: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     ZOIDFS_PATH_MAX +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, (void *)sattr);
    } else {
        null_param = 0;
        sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_null_param_t,
                                &null_param) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                                (void *)parent_handle) +
                     ZOIDFS_NAME_MAX +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_sattr_t, (void *)sattr);
    }

    if (parent_hint) {
        recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t,
                                 &op_status) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t,
                                parent_hint);
    } else {
        recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t,
                                 &op_status) +
                     xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, &hint);
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_mkdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_mkdir: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        fprintf(stderr, "zoidfs_mkdir: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            fprintf(stderr, "zoidfs_mkdir: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            fprintf(stderr, "zoidfs_mkdir: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            fprintf(stderr, "zoidfs_mkdir: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (!xdr_zoidfs_sattr_t(&xdrs, (zoidfs_sattr_t *)sattr)) {
        fprintf(stderr, "zoidfs_mkdir: xdr_zoidfs_sattr_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_mkdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_mkdir: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, parent_hint)) {
            fprintf(stderr,
                    "zoidfs_mkdir: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_readdir
 * This function returns the dirents from the specified parent directory. The
 * cookie is a pointer which specifies where in the directory to start
 * fetching the dirents from.
 */
int zoidfs_readdir(const zoidfs_handle_t *parent_handle,
                   zoidfs_dirent_cookie_t cookie, size_t *entry_count,
                   zoidfs_dirent_t *entries, uint32_t flags,
                   zoidfs_cache_hint_t *parent_hint) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    zoidfs_cache_hint_t hint;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_READDIR;

    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 xdr_sizeof((xdrproc_t)xdr_u_long, entry_count) +
                 *entry_count * xdr_sizeof((xdrproc_t)xdr_zoidfs_dirent_t,
                                           entries) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_cache_hint_t, &hint);

    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t,
                            (void *)parent_handle) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_dirent_cookie_t, &cookie) +
                 xdr_sizeof((xdrproc_t)xdr_u_long, entry_count) +
                 xdr_sizeof((xdrproc_t)xdr_uint32_t, &flags);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_readdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_readdir: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
        fprintf(stderr, "zoidfs_readdir: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_dirent_cookie_t(&xdrs, &cookie)) {
        fprintf(stderr,
                "zoidfs_readdir: xdr_zoidfs_dirent_cookie_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_long(&xdrs, entry_count)) {
        fprintf(stderr, "zoidfs_readdir: xdr_u_long() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_uint32_t(&xdrs, &flags)) {
        fprintf(stderr, "zoidfs_readdir: xdr_uint32_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_readdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_readdir: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_long(&xdrs, entry_count)) {
        fprintf(stderr, "zoidfs_readdir: xdr_u_long() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&entries, (unsigned int *)&entry_count,
                   MAXARRSIZE, sizeof(zoidfs_dirent_t),
                   (xdrproc_t)xdr_zoidfs_dirent_t)) {
        fprintf(stderr, "zoidfs_readdir: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, parent_hint)) {
            fprintf(stderr,
                    "zoidfs_readdir: xdr_zoidfs_cache_hint_t() failed.\n");
            return ZFSERR_XDR;
        }
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_resize
 * This function resizes the file associated with the file handle.
 */
int zoidfs_resize(const zoidfs_handle_t *handle, uint64_t size) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_RESIZE;

    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status);
    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (void *)handle) +
                 xdr_sizeof((xdrproc_t)xdr_uint64_t, &size);

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_resize: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_resize: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_resize: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_uint64_t(&xdrs, &size)) {
        fprintf(stderr, "zoidfs_resize: xdr_uint64_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_resize: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_resize: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_write
 * This function implements the zoidfs write call.
 */
int zoidfs_write(const zoidfs_handle_t *handle, size_t mem_count,
                 const void *mem_starts[], const size_t mem_sizes[],
                 size_t file_count, const uint64_t file_starts[],
                 uint64_t file_sizes[]) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_WRITE;

    /*
     * TODO I need to add a constant 1024 to both sendbuflen and recvbuflen
     * so that xdr_array doesn't fail. This needs further investigating.
     */
    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 file_count * xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 1024;
    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (void *)handle) +
                 xdr_sizeof((xdrproc_t)xdr_u_long, &mem_count) +
                 mem_count * xdr_sizeof((xdrproc_t)xdr_u_long, &mem_count) +
                 xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 file_count * xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 file_count * xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 1024;

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_write: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /*
     * Encode the function parameters using XDR. We will NOT encode
     * the data.
     */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_write: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_write: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_long(&xdrs, &mem_count)) {
        fprintf(stderr, "zoidfs_write: xdr_u_long() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&mem_sizes, (unsigned int *)&mem_count,
                   INT_MAX, sizeof(size_t), (xdrproc_t)xdr_u_long)) {
        fprintf(stderr, "zoidfs_write: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_long(&xdrs, &file_count)) {
        fprintf(stderr, "zoidfs_write: xdr_u_long() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_starts, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_uint64_t)) {
        fprintf(stderr, "zoidfs_write: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_uint64_t)) {
        fprintf(stderr, "zoidfs_write: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    /* Send the data using an expected BMI message */
    sendbuflen = mem_count * ZOIDFS_BUFFER_MAX;
    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_write: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    if (mem_count == 1) {
        /* Contiguous write */
        ret = bmi_comm_send(peer_addr, (void *)mem_starts[0], mem_sizes[0],
                            tag, context);
    } else {
        /* TODO Strided writes. Aggregate the individual buffers */
        ret = bmi_comm_send(peer_addr, sendbuf, sendbuflen, tag, context);
    }

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_write: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_write: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_uint64_t)) {
        fprintf(stderr, "zoidfs_write: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
int zoidfs_read(const zoidfs_handle_t *handle, size_t mem_count,
                void *mem_starts[], const size_t mem_sizes[],
                size_t file_count, const uint64_t file_starts[],
                uint64_t file_sizes[]) {
    int ret;
    XDR xdrs;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_PROTO_READ;

    /*
     * TODO I need to add a constant 1024 to both sendbuflen and recvbuflen
     * so that xdr_array doesn't fail. This needs further investigating.
     */
    sendbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_id_t, &zoidfs_op_id) +
                 xdr_sizeof((xdrproc_t)xdr_zoidfs_handle_t, (void *)handle) +
                 xdr_sizeof((xdrproc_t)xdr_u_long, &mem_count) +
                 mem_count * xdr_sizeof((xdrproc_t)xdr_u_long, &mem_count) +
                 xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 file_count * xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 file_count * xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 1024;

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "zoidfs_read: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    /*
     * Encode the function parameters using XDR. We will NOT encode/decode
     * the data.
     */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        fprintf(stderr, "zoidfs_read: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        fprintf(stderr, "zoidfs_read: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_long(&xdrs, &mem_count)) {
        fprintf(stderr, "zoidfs_read: xdr_u_long() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&mem_sizes, (unsigned int *)&mem_count,
                   INT_MAX, sizeof(size_t), (xdrproc_t)xdr_u_long)) {
        fprintf(stderr, "zoidfs_read: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_long(&xdrs, &file_count)) {
        fprintf(stderr, "zoidfs_read: xdr_u_long() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_starts, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_uint64_t)) {
        fprintf(stderr, "zoidfs_read: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_uint64_t)) {
        fprintf(stderr, "zoidfs_read: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /*
     * Send the encoded function parameters to the ION daemon using an
     * unexpected BMI message.
     */
    ret = bmi_comm_sendu(peer_addr, sendbuf, sendbuflen, tag, context);

    /* Receive the data from the IOD */
    recvbuflen = mem_count * ZOIDFS_BUFFER_MAX;
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_read: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    if (mem_count == 1) {
        /* Contiguous read */
        ret = bmi_comm_recv(peer_addr, mem_starts[0], ZOIDFS_BUFFER_MAX, tag,
                            context);
    } else {
        /* TODO Strided reads. Decompose recvbuf into individual buffers */
        ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);
    }
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);

    /* Wait for the response from the ION */
    recvbuflen = xdr_sizeof((xdrproc_t)xdr_zoidfs_op_status_t, &op_status) +
                 file_count * xdr_sizeof((xdrproc_t)xdr_u_long, &file_count) +
                 1024;
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "zoidfs_read: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }

    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        fprintf(stderr, "zoidfs_read: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_uint64_t)) {
        fprintf(stderr, "zoidfs_read: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    xdr_destroy(&xdrs);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);

    return op_status;
}


/*
 * zoidfs_init
 * Initialize the client subsystems.
 */
int zoidfs_init(void) {
    int ret;

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
        fprintf(stderr, "zoidfs_init: getenv() failed.\n");
        exit(1);
    }

    /* Perform an address lookup on the ION */
    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        fprintf(stderr, "zoidfs_init: BMI_addr_lookup() failed.\n");
        exit(1);
    }

    return 0;
}


/*
 * zoidfs_finalize
 * Finalize the client subsystems.
 */
int zoidfs_finalize(void) {
    int ret;

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
