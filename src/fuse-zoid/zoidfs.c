/*
 * zoidfs.c
 * Client-side implementation of the ZOIDFS API. The CNs communicate with
 * the IONs over BMI.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#include "bmi.h"
#include "zoidfs.h"
#include "gossip.h"
#include "bmi_comm.h"
#include "zoidfs_xdr.h"
#include "zoidfs_ops.h"

static char *ion_name;
static bmi_context_id context;

/*
 * zoidfs_null
 * This function implements a noop operation. The IOD returns a 1-byte message
 * to the CN.
 */
void zoidfs_null(void) {
    int ret;
    XDR xdrs;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_NULL;

    recvbuflen = 1;
    sendbuflen = sizeof(zoidfs_op_id_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_null: BMI_addr_lookup() failed.\n");
        exit(1);
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_null: BMI_memalloc() failed.\n");
        exit(1);
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_null: xdr_zoidfs_op_id_t() failed.\n");
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
        gossip_err("zoidfs_null: BMI_memalloc() failed.\n");
        exit(1);
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Free up the message buffers */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);
    BMI_memfree(peer_addr, sendbuf, sendbuflen, BMI_SEND);
}


/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle from
 * the ION.
 */
int zoidfs_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr) {
    int ret;
    XDR xdrs;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_GET_ATTR;

    sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_handle_t);
    recvbuflen = sizeof(zoidfs_op_status_t) + sizeof(zoidfs_attr_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_getattr: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_getattr: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_getattr: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_getattr: xdr_zoidfs_handle_t() failed.\n");
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
        gossip_err("zoidfs_getattr: BMI_memalloc() failed.\n");
        return ZFSERR_XDR;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_getattr: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_attr_t(&xdrs, attr)) {
        gossip_err("zoidfs_getattr: xdr_zoidfs_attr_t() failed.\n");
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
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_SET_ATTR;

    recvbuflen = sizeof(zoidfs_op_status_t) + sizeof(zoidfs_attr_t);
    sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_handle_t) +
                 sizeof(zoidfs_sattr_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_setattr: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_setattr: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_setattr: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_setattr: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_sattr_t(&xdrs, (zoidfs_sattr_t *)sattr)) {
        gossip_err("zoidfs_setattr: xdr_zoidfs_sattr_t() failed.\n");
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
        gossip_err("zoidfs_setattr: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_setattr: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (attr) {
        if (!xdr_zoidfs_attr_t(&xdrs, attr)) {
            gossip_err("zoidfs_setattr: xdr_zoidfs_attr_t() failed.\n");
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
    return -ENOSYS;
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
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_LOOKUP;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        gossip_err("zoidfs_lookup: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     ZOIDFS_PATH_MAX;
    } else {
        null_param = 0;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     sizeof(zoidfs_handle_t) + ZOIDFS_NAME_MAX;
    }
    recvbuflen = sizeof(zoidfs_op_status_t) + sizeof(zoidfs_handle_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_lookup: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_lookup: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_lookup: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        gossip_err("zoidfs_lookup: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            gossip_err("zoidfs_lookup: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            gossip_err("zoidfs_lookup: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            gossip_err("zoidfs_lookup: xdr_string() failed.\n");
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
        gossip_err("zoidfs_lookup: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_lookup: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_lookup: xdr_zoidfs_handle_t() failed.\n");
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
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_REMOVE;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        gossip_err("zoidfs_remove: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     ZOIDFS_PATH_MAX;
    } else {
        null_param = 0;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     sizeof(zoidfs_handle_t) + ZOIDFS_NAME_MAX;
    }
    recvbuflen = sizeof(zoidfs_op_status_t) + sizeof(zoidfs_cache_hint_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_remove: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_remove: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_remove: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        gossip_err("zoidfs_remove: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            gossip_err("zoidfs_remove: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            gossip_err("zoidfs_remove: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            gossip_err("zoidfs_remove: xdr_string() failed.\n");
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
        gossip_err("zoidfs_remove: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_remove: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, parent_hint)) {
            gossip_err("zoidfs_remove: xdr_zoidfs_cache_hint_t() failed.\n");
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
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_COMMIT;

    recvbuflen = sizeof(zoidfs_op_status_t);
    sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_handle_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_commit: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_commit: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_commit: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_commit: xdr_zoidfs_handle_t() failed.\n");
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
        gossip_err("zoidfs_commit: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_commit: xdr_zoidfs_op_status_t() failed.\n");
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
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_CREATE;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        gossip_err("zoidfs_create: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     ZOIDFS_PATH_MAX + sizeof(zoidfs_sattr_t);
    } else {
        null_param = 0;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     sizeof(zoidfs_handle_t) + ZOIDFS_NAME_MAX +
                     sizeof(zoidfs_sattr_t);
    }

    recvbuflen = sizeof(zoidfs_op_status_t) + sizeof(zoidfs_handle_t) +
                 sizeof(int);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_create: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_create: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_create: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        gossip_err("zoidfs_create: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            gossip_err("zoidfs_create: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            gossip_err("zoidfs_create: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            gossip_err("zoidfs_create: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (!xdr_zoidfs_sattr_t(&xdrs, (zoidfs_sattr_t *)sattr)) {
        gossip_err("zoidfs_create: xdr_zoidfs_sattr_t() failed.\n");
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
        gossip_err("zoidfs_create: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_create: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_create: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }

    /* Decode the "created" field only if the caller wants it */
    if (created) {
        if (!xdr_int(&xdrs, created)) {
            gossip_err("zoidfs_create: xdr_int() failed.\n");
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
    return -ENOSYS;
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
                   const char *to_full_path, const zoidfs_sattr_t *sattr,
                   zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs_cache_hint_t *to_parent_hint) {
    return -ENOSYS;
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
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    zoidfs_null_param_t null_param;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_MKDIR;

    /*
     * Check for invalid path params. The caller should either specify the
     * full_path or specify the parent_handle AND the component_name.
     */
    if ((!parent_handle || !component_name) && !full_path) {
        gossip_err("zoidfs_mkdir: Invalid path parameters.\n");
        return ZFSERR_MISC;
    }

    /*
     * The null_param informs the server whether the client is passing the full
     * path of the object or the parent_handle and component_name. This is
     * required for decoding the parameters correctly on the server.
     */
    if (full_path) {
        null_param = 1;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     ZOIDFS_PATH_MAX + sizeof(zoidfs_sattr_t);
    } else {
        null_param = 0;
        sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_null_param_t) +
                     sizeof(zoidfs_handle_t) + ZOIDFS_NAME_MAX +
                     sizeof(zoidfs_sattr_t);
    }

    recvbuflen = sizeof(zoidfs_op_status_t) + sizeof(zoidfs_cache_hint_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_mkdir: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_mkdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_mkdir: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_null_param_t(&xdrs, &null_param)) {
        gossip_err("zoidfs_mkdir: xdr_zoidfs_null_param_t() failed.\n");
        return ZFSERR_XDR;
    }

    if (full_path) {
        if (!xdr_string(&xdrs, (char **)&full_path, ZOIDFS_PATH_MAX)) {
            gossip_err("zoidfs_mkdir: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    } else {
        if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
            gossip_err("zoidfs_mkdir: xdr_zoidfs_handle_t() failed.\n");
            return ZFSERR_XDR;
        }
        if (!xdr_string(&xdrs, (char **)&component_name, ZOIDFS_NAME_MAX)) {
            gossip_err("zoidfs_mkdir: xdr_string() failed.\n");
            return ZFSERR_XDR;
        }
    }

    if (!xdr_zoidfs_sattr_t(&xdrs, (zoidfs_sattr_t *)sattr)) {
        gossip_err("zoidfs_mkdir: xdr_zoidfs_sattr_t() failed.\n");
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
        gossip_err("zoidfs_mkdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_mkdir: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, parent_hint)) {
            gossip_err("zoidfs_mkdir: xdr_zoidfs_cache_hint_t() failed.\n");
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
                   zoidfs_dirent_cookie_t cookie, int *entry_count,
                   zoidfs_dirent_t *entries,
                   zoidfs_cache_hint_t *parent_hint) {
    int ret;
    XDR xdrs;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_READDIR;

    recvbuflen = sizeof(zoidfs_op_status_t) + sizeof(int) +
                 *entry_count * sizeof(zoidfs_dirent_t) +
                 sizeof(zoidfs_cache_hint_t);
    sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_handle_t) +
                 sizeof(zoidfs_dirent_cookie_t) + sizeof(int);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_readdir: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_readdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_readdir: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, parent_handle)) {
        gossip_err("zoidfs_readdir: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_dirent_cookie_t(&xdrs, &cookie)) {
        gossip_err("zoidfs_readdir: xdr_zoidfs_dirent_cookie_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_int(&xdrs, entry_count)) {
        gossip_err("zoidfs_readdir: xdr_int() failed.\n");
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
        gossip_err("zoidfs_readdir: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_readdir: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_int(&xdrs, entry_count)) {
        gossip_err("zoidfs_readdir: xdr_int() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&entries, (unsigned int *)&entry_count,
                   MAXARRSIZE, sizeof(zoidfs_dirent_t),
                   (xdrproc_t)xdr_zoidfs_dirent_t)) {
        gossip_err("zoidfs_readdir: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (parent_hint) {
        if (!xdr_zoidfs_cache_hint_t(&xdrs, parent_hint)) {
            gossip_err("zoidfs_readdir: xdr_zoidfs_cache_hint_t() failed.\n");
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
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_RESIZE;

    recvbuflen = sizeof(zoidfs_op_status_t);
    sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_handle_t) +
                 sizeof(uint64_t);

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_resize: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_resize: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /* Encode the function parameters using XDR */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_resize: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_resize: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_u_longlong_t(&xdrs, &size)) {
        gossip_err("zoidfs_resize: xdr_u_longlong_t() failed.\n");
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
        gossip_err("zoidfs_resize: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    /* Do a BMI receive in recvbuf */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_resize: xdr_zoidfs_op_status_t() failed.\n");
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
int zoidfs_write(const zoidfs_handle_t *handle, int mem_count,
                 const void *mem_starts[], const size_t mem_sizes[],
                 int file_count, const uint64_t file_starts[],
                 uint64_t file_sizes[]) {
    int ret;
    XDR xdrs;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_WRITE;

    /*
     * TODO I need to add a constant 1024 to both sendbuflen and recvbuflen
     * so that xdr_array doesn't fail. This needs further investigating.
     */
    recvbuflen = sizeof(zoidfs_op_status_t) + file_count * sizeof(uint64_t)
                 + 1024;
    sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_handle_t) +
                 sizeof(int) + mem_count * sizeof(size_t) +
                 sizeof(int) + file_count * sizeof(uint64_t) +
                 file_count * sizeof(uint64_t) + 1024;

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_write: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_write: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /*
     * Encode the function parameters using XDR. We will NOT encode
     * the data.
     */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_write: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_write: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_int(&xdrs, &mem_count)) {
        gossip_err("zoidfs_write: xdr_int() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&mem_sizes, (unsigned int *)&mem_count,
                   INT_MAX, sizeof(size_t), (xdrproc_t)xdr_u_int)) {
        gossip_err("zoidfs_write: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_int(&xdrs, &file_count)) {
        gossip_err("zoidfs_write: xdr_int() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_starts, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_u_longlong_t)) {
        gossip_err("zoidfs_write: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_u_longlong_t)) {
        gossip_err("zoidfs_write: xdr_array() failed.\n");
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
        gossip_err("zoidfs_write: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    if (mem_count == 1) {
        /* Contiguous write */
        memcpy(sendbuf, mem_starts[0], mem_sizes[0]);
    } else {
        /* TODO Strided writes. Aggregate the individual buffers */
    }

    ret = bmi_comm_send(peer_addr, sendbuf, sendbuflen, tag+1, context);

    /* Wait for the response from the ION */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        gossip_err("zoidfs_write: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+2, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_write: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_u_longlong_t)) {
        gossip_err("zoidfs_write: xdr_array() failed.\n");
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
int zoidfs_read(const zoidfs_handle_t *handle, int mem_count,
                void *mem_starts[], const size_t mem_sizes[], int file_count,
                const uint64_t file_starts[], uint64_t file_sizes[]) {
    int ret;
    XDR xdrs;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    void *sendbuf, *recvbuf;
    bmi_size_t sendbuflen, recvbuflen;
    zoidfs_op_status_t op_status = ZFS_OK;
    zoidfs_op_id_t zoidfs_op_id = ZOIDFS_READ;

    /*
     * TODO I need to add a constant 1024 to both sendbuflen and recvbuflen
     * so that xdr_array doesn't fail. This needs further investigating.
     */
    sendbuflen = sizeof(zoidfs_op_id_t) + sizeof(zoidfs_handle_t) +
                 sizeof(int) + mem_count * sizeof(size_t) +
                 sizeof(int) + file_count * sizeof(uint64_t) +
                 file_count * sizeof(uint64_t) + 1024;

    ret = BMI_addr_lookup(&peer_addr, ion_name);
    if (ret < 0) {
        gossip_err("zoidfs_read: BMI_addr_lookup() failed.\n");
        return ZFSERR_MISC;
    }

    sendbuf = BMI_memalloc(peer_addr, sendbuflen, BMI_SEND);
    if (!sendbuf) {
        gossip_err("zoidfs_read: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(sendbuf, 0, sendbuflen);

    /*
     * Encode the function parameters using XDR. We will NOT encode/decode
     * the data.
     */
    xdrmem_create(&xdrs, sendbuf, sendbuflen, XDR_ENCODE);
    if (!xdr_zoidfs_op_id_t(&xdrs, &zoidfs_op_id)) {
        gossip_err("zoidfs_read: xdr_zoidfs_op_id_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_zoidfs_handle_t(&xdrs, handle)) {
        gossip_err("zoidfs_read: xdr_zoidfs_handle_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_int(&xdrs, &mem_count)) {
        gossip_err("zoidfs_read: xdr_int() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&mem_sizes, (unsigned int *)&mem_count,
                   INT_MAX, sizeof(size_t), (xdrproc_t)xdr_u_int)) {
        gossip_err("zoidfs_read: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_int(&xdrs, &file_count)) {
        gossip_err("zoidfs_read: xdr_int() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_starts, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_u_longlong_t)) {
        gossip_err("zoidfs_read: xdr_array() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_u_longlong_t)) {
        gossip_err("zoidfs_read: xdr_array() failed.\n");
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
        gossip_err("zoidfs_read: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);

    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+1, context);

    if (mem_count == 1) {
        /* Contiguous read */
        memcpy(mem_starts[0], recvbuf, mem_sizes[0]);
    } else {
        /* TODO Strided reads. Decompose recvbuf into individual buffers */
    }
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);

    /* Wait for the response from the ION */
    recvbuflen = sizeof(zoidfs_op_status_t) + file_count * sizeof(uint64_t)
                 + 1024;
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        gossip_err("zoidfs_read: BMI_memalloc() failed.\n");
        return ZFSERR_MISC;
    }
    memset(recvbuf, 0, recvbuflen);
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+2, context);

    /* Decode the ION response */
    xdrmem_create(&xdrs, recvbuf, recvbuflen, XDR_DECODE);
    if (!xdr_zoidfs_op_status_t(&xdrs, &op_status)) {
        gossip_err("zoidfs_read: xdr_zoidfs_op_status_t() failed.\n");
        return ZFSERR_XDR;
    }
    if (!xdr_array(&xdrs, (char **)&file_sizes, (unsigned int *)&file_count,
                   INT_MAX, sizeof(uint64_t), (xdrproc_t)xdr_u_longlong_t)) {
        gossip_err("zoidfs_read: xdr_array() failed.\n");
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

    /* Initialize BMI */
    ret = BMI_initialize(NULL, NULL, 0);
    if (ret < 0) {
        gossip_err("zoidfs_init: BMI_initialize() failed.\n");
        exit(1);
    }

    /* Create a new BMI context */
    ret = BMI_open_context(&context);
    if (ret < 0) {
        gossip_err("zoidfs_init: BMI_open_context() failed.\n");
        exit(1);
    }

    /*
     * Pick up the ION hostname from an environment variable (ZOIDFS_ION_NAME).
     */
    ion_name = getenv(ION_ENV);
    if (!ion_name) {
        gossip_err("zoidfs_init: getenv() failed.\n");
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
        gossip_err("zoidfs_finalize: BMI_finalize() failed.\n");
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
