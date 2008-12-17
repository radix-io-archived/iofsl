#include "bmi.h"
#include "bmi_comm.h"

#include <stdio.h>
#include <errno.h>

int bmi_client_init(void);
int bmi_client_finalize(void);
int bmi_client_handshake(BMI_addr_t, bmi_msg_tag_t, bmi_context_id);

int main(int argc, char **argv) {
    int ret;
    BMI_addr_t peer_addr;
    bmi_msg_tag_t tag = 0;
    bmi_context_id context;
    char str[] = "bmi-client: hello world\n";

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        exit(1);
    }

    ret = bmi_client_init();

    ret = BMI_addr_lookup(&peer_addr, argv[1]);
    if (ret < 0) {
        errno = -ret;
        perror("bmi-client: BMI_addr_lookup()");
        exit(1);
    }

    ret = BMI_open_context(&context);
    if (ret < 0) {
        errno = -ret;
        perror("bmi-client: BMI_open_context()");
        exit(1);
    }

    ret = bmi_client_handshake(peer_addr, tag, context);

    ret = bmi_comm_send(peer_addr, str, strlen(str)+1, tag+2, context);

    BMI_close_context(context);

    ret = bmi_client_finalize();

    return 0;
}


int bmi_client_init(void) {
    int ret;

    /* Initialize BMI */
    ret = BMI_initialize(NULL, NULL, 0);
    if (ret < 0) {
        errno = -ret;
        perror("bmi-client: BMI_initialize()");
        exit(1);
    }
    return 0;
}


int bmi_client_finalize(void) {
    int ret;

    /* Finalize BMI */
    ret = BMI_finalize();
    if (ret < 0) {
        errno = -ret;
        perror("bmi-client: BMI_finalize()");
        exit(1);
    }
    return 0;
}


int bmi_client_handshake(BMI_addr_t peer_addr, bmi_msg_tag_t tag,
                             bmi_context_id context) {
    int ret;
    int max_bytes = 4;
    void *sendbuf, *recvbuf;
    int zoidfs_op_id = 9;

    /* Allocate memory for send and recv buf */
    sendbuf = BMI_memalloc(peer_addr, max_bytes, BMI_SEND);
    if (!sendbuf) {
        fprintf(stderr, "bmi_client_handshake: BMI_memalloc() failed.\n");
        exit(1);
    }
    memset(sendbuf, 0, max_bytes);

    recvbuf = BMI_memalloc(peer_addr, max_bytes, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "bmi_client_handshake: BMI_memalloc() failed.\n");
        exit(1);
    }
    memset(recvbuf, 0, max_bytes);

    /* Send the test buffer to the server */
    memcpy(sendbuf, &zoidfs_op_id, max_bytes);
    ret = bmi_comm_sendu(peer_addr, sendbuf, max_bytes, tag, context);

    /* Recv the ack from the server */
    ret = bmi_comm_recv(peer_addr, recvbuf, max_bytes, tag+1, context);

    /* free up the message buffers */
    BMI_memfree(peer_addr, sendbuf, max_bytes, BMI_SEND);
    BMI_memfree(peer_addr, recvbuf, max_bytes, BMI_RECV);

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
