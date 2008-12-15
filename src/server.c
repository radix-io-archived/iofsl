/*
 * iod.c
 * I/O daemon. The CNs communicate with the IONs over BMI.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#include "iod.h"

int32_t main(int32_t argc, char **argv) {
    int32_t ret;
    void *recvbuf;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    bmi_context_id context;
    bmi_size_t recvbuflen = BUFSIZE;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(1);
    }

    /* Initialize BMI */
    ret = iod_bmi_init(argv[1]);

    ret = BMI_open_context(&context);
    if (ret < 0) {
        errno = -ret;
        perror("iod: BMI_open_context()");
        exit(1);
    }

    ret = iod_bmi_handshake(&peer_addr, tag, context);

    /* Create a buffer to recv into */
    recvbuf = BMI_memalloc(peer_addr, recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "iod: BMI_memalloc() failed.\n");
        exit(1);
    }
    memset(recvbuf, 0, recvbuflen);

    /* Recv the data from the client */
    ret = bmi_comm_recv(peer_addr, recvbuf, recvbuflen, tag+2, context);

    printf("%s", (char *)recvbuf);

    /* Cleanup */
    BMI_memfree(peer_addr, recvbuf, recvbuflen, BMI_RECV);

    /* Finalize BMI */
    BMI_close_context(context);
    ret = iod_bmi_finalize();

    return 0;
}


int32_t iod_bmi_init(char *hostid) {
    int32_t ret;
    char *network = NULL;

    /* get the network from the hostid, e.g "bmi_tcp" */
    iod_get_network(hostid, &network);

    /* Initialize BMI */
    ret = BMI_initialize(network, hostid, BMI_INIT_SERVER);
    if (ret < 0) {
        errno = -ret;
        perror("iod: BMI_initialize()");
        exit(1);
    }
    return 0;
}


int32_t iod_bmi_finalize(void) {
    int32_t ret;

    /* Finalize BMI */
    ret = BMI_finalize();
    if (ret < 0) {
        errno = -ret;
        perror("iod: BMI_finalize()");
        exit(1);
    }
    return 0;
}


static void iod_get_network(char *id, char **network) {
    if (id[0] == 't' && id[1] == 'c' && id[2] == 'p' && iod_check_uri(&id[3])) {
        *network = strdup("bmi_tcp");
    } else if (id[0] == 'g' && id[1] == 'm' && iod_check_uri(&id[2])) {
        *network = strdup("bmi_gm");
    } else if (id[0] == 'm' && id[1] == 'x' && iod_check_uri(&id[2])) {
        *network = strdup("bmi_mx");
    } else if (id[0] == 'i' && id[1] == 'b' && iod_check_uri(&id[2])) {
        *network = strdup("bmi_ib");
    }
    return;
}


static int32_t iod_check_uri(char *uri) {
    int32_t ret = 0; /* failure */
    if (uri[0] == ':' && uri[1] == '/' && uri[2] == '/') ret = 1;
    return ret;
}


int32_t iod_bmi_handshake(BMI_addr_t *peer_addr, bmi_msg_tag_t tag,
                          bmi_context_id context) {
    void *sendbuf = NULL;
    int32_t max_bytes = 4;
    int32_t ret, outcount = 0;
    struct BMI_unexpected_info request_info;

    /* Wait for an initial request from client */
    do {
        ret = BMI_testunexpected(1, &outcount, &request_info, MAX_IDLE_TIME);
    } while (ret == 0 && outcount == 0);

    if (ret < 0) {
        fprintf(stderr,
                "iod_bmi_handshake: Request recv failure (bad state).\n");
        errno = -ret;
        perror("iod_bmi_handshake: BMI_testunexpected");
        exit(1);
    }

    if (request_info.error_code != 0) {
        fprintf(stderr,
                "iod_bmi_handshake: Request recv failure (bad state).\n");
        exit(1);
    }

    *peer_addr = request_info.addr;

    printf("zoidfs_op_id: %d\n", *(int32_t *)request_info.buffer);
    printf("tag: %d\n", request_info.tag);

    BMI_unexpected_free(*peer_addr, request_info.buffer);

    /* create an ack */
    sendbuf = BMI_memalloc(*peer_addr, max_bytes, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "iod_bmi_handshake: BMI_memalloc failed().\n");
        exit(1);
    }
    memset(sendbuf, 0, max_bytes);

    /* post the ack */
    ret = bmi_comm_send(*peer_addr, sendbuf, max_bytes, tag+1, context);

    /* free up the message buffers */
    BMI_memfree(*peer_addr, sendbuf, max_bytes, BMI_SEND);

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
