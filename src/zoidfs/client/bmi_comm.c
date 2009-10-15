/*
 * bmi_comm.c
 * Synchronous BMI calls. BMI communication is inherently asynchronous. We use
 * wrapper functions to create synchronous send/recv calls.
 *
 * Nawab Ali <alin@cse.ohio-state.edu
 * Jason Cope <copej@mcs.anl.gov>
 */

#include "bmi_comm.h"

/*
 * bmi_comm_send
 * Synchronous call for sending messages using BMI.
 */
int bmi_comm_send(BMI_addr_t peer_addr, const void *buffer, bmi_size_t buflen,
                  bmi_msg_tag_t tag, bmi_context_id context) {
    bmi_op_id_t op_id;
    int ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    /* Post the BMI send request and wait for its completion */
    ret = BMI_post_send(&op_id, peer_addr, buffer, buflen, BMI_PRE_ALLOC, tag,
                        NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_send: BMI_post_send() failed.\n");
        exit(1);
    } else if (ret == 0) {
        do {
            ret = BMI_test(op_id, &outcount, &error_code, &actual_size, NULL,
                           MAX_IDLE_TIME, context);
        } while (ret == 0 && outcount == 0);

        if (ret < 0 || error_code != 0) {
            fprintf(stderr, "bmi_comm_send: Data send failed.\n");
            exit(1);
        }

        if (actual_size != buflen) {
            fprintf(stderr, "bmi_comm_send: Expected %ld but received %lu\n",
                    buflen, actual_size);
            exit(1);
        }
    }

    return 0;
}

/*
 * zoidfs wrapper for BMI_post_send... includes error handling
 */
int bmi_comm_isend(BMI_addr_t peer_addr, const void *buffer, bmi_size_t buflen,
                  bmi_msg_tag_t tag, bmi_context_id context, bmi_op_id_t * op_id) {
    int ret = 0;

    /* Post the BMI send request */
    ret = BMI_post_send(op_id, peer_addr, buffer, buflen, BMI_PRE_ALLOC, tag,
                        NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_isend: BMI_post_send() failed.\n");
        exit(1);
    }
    
    return ret ? 1 : 0;
}

/*
 * zoidfs wrapper for BMI_test to complement a BMI_post_send... includes error handling
 */
int bmi_comm_isend_wait(bmi_op_id_t op_id, bmi_size_t buflen, bmi_context_id context)
{
    int ret = 0, outcount = 0;
    bmi_size_t actual_size = 0;
    bmi_error_code_t error_code;

    do {
        ret = BMI_test(op_id, &outcount, &error_code, &actual_size, NULL, MAX_IDLE_TIME, context);
    } while (ret == 0 && outcount == 0);

    if (ret < 0 || error_code != 0)
    {
        fprintf(stderr, "bmi_comm_wait: Data send failed.\n");
        exit(1);
    }

    if (actual_size != buflen)
    {
        fprintf(stderr, "bmi_comm_wait: Expected %ld but received %lu\n", buflen, actual_size);
        exit(1);
    }
    return 0;
}


/*
 * bmi_comm_recv
 * Synchronous call for receiving messages using BMI.
 */
int bmi_comm_recv(BMI_addr_t peer_addr, void *buffer, bmi_size_t buflen,
                  bmi_msg_tag_t tag, bmi_context_id context, bmi_size_t * actual_size) {
    bmi_op_id_t op_id;
    int ret, outcount;
    bmi_error_code_t error_code;

    /* Post the BMI recv request and wait for its completion */
    ret = BMI_post_recv(&op_id, peer_addr, buffer, buflen, actual_size,
                        BMI_PRE_ALLOC, tag, NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_recv: BMI_post_recv() failed.\n");
        exit(1);
    } else if (ret == 0) {
        do {
            ret = BMI_test(op_id, &outcount, &error_code, actual_size, NULL,
                           MAX_IDLE_TIME, context);
        } while (ret == 0 && outcount == 0);

        if (ret < 0 || error_code != 0) {
            fprintf(stderr, "bmi_comm_recv: Data receive failed.\n");
            exit(1);
        }
    }

    return 0;
}

/*
 * zoidfs wrapper for BMI_test to complement a BMI_post_recv... includes error handling
 */
int bmi_comm_irecv_wait(bmi_op_id_t op_id, bmi_size_t * actual_size, bmi_context_id context)
{
    int ret = 0, outcount = 0;
    bmi_error_code_t error_code;

    do {
        ret = BMI_test(op_id, &outcount, &error_code, actual_size, NULL, MAX_IDLE_TIME, context);
    } while (ret == 0 && outcount == 0);

    if (ret < 0 || error_code != 0)
    {
        fprintf(stderr, "bmi_comm_wait: Data send failed.\n");
        exit(1);
    }

    return 0;
}

/*
 * zoidfs wrapper for BMI_post_recv... includes error handling
 */
int bmi_comm_irecv(BMI_addr_t peer_addr, void *buffer, bmi_size_t buflen,
                  bmi_msg_tag_t tag, bmi_context_id context, bmi_size_t * actual_size, bmi_op_id_t * op_id) {
    int ret = 0;

    /* Post the BMI recv request and wait for its completion */
    ret = BMI_post_recv(op_id, peer_addr, buffer, buflen, actual_size,
                        BMI_PRE_ALLOC, tag, NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_irecv: BMI_post_recv() failed.\n");
        exit(1);
    }

    return 0;
}

/*
 * bmi_comm_sendu
 * Synchronous call for sending unexpected messages using BMI.
 */
int bmi_comm_sendu(BMI_addr_t peer_addr, const void *buffer, bmi_size_t buflen,
                   bmi_msg_tag_t tag, bmi_context_id context) {
    bmi_op_id_t op_id;
    int ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    /* Post the BMI unexpected send request and wait for its completion */
    ret = BMI_post_sendunexpected(&op_id, peer_addr, buffer, buflen,
                                  BMI_PRE_ALLOC, tag, NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_sendu: BMI_post_sendunexpected() failed.\n");
        exit(1);
    } else if (ret == 0) {
        do {
            ret = BMI_test(op_id, &outcount, &error_code, &actual_size, NULL,
                           MAX_IDLE_TIME, context);
        } while (ret == 0 && outcount == 0);

        if (ret < 0 || error_code != 0) {
            fprintf(stderr, "bmi_comm_sendu: Data send failed.\n");
            exit(1);
        }

        if (actual_size != buflen) {
            fprintf(stderr, "bmi_comm_sendu: Expected %ld but received %lu\n",
                    buflen, actual_size);
            exit(1);
        }
    }

    return 0;
}

int bmi_comm_isendu(BMI_addr_t peer_addr, const void *buffer, bmi_size_t buflen,
                   bmi_msg_tag_t tag, bmi_context_id context, bmi_op_id_t * op_id) {
    int ret = 0;

    /* Post the BMI unexpected send request and wait for its completion */
    ret = BMI_post_sendunexpected(op_id, peer_addr, buffer, buflen,
                                  BMI_PRE_ALLOC, tag, NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_isendu: BMI_post_sendunexpected() failed.\n");
        exit(1);
    }

    /* immediate bmi completion detected */
    return ret ? 1 : 0;
}

int bmi_comm_isendu_wait(bmi_size_t buflen, bmi_context_id context, bmi_op_id_t op_id)
{
    int ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    do {
        ret = BMI_test(op_id, &outcount, &error_code, &actual_size, NULL,
        MAX_IDLE_TIME, context);
    } while (ret == 0 && outcount == 0);

    if (ret < 0 || error_code != 0) {
        fprintf(stderr, "bmi_comm_isendu_wait: Data send failed.\n");
        exit(1);
    }

    if (actual_size != buflen)
    {
        fprintf(stderr, "bmi_comm_isendu_wait: Expected %ld but received %lu\n", buflen, actual_size);
        exit(1);
    }
    return 0;
}

/*
 * bmi_comm_recvu
 * Synchronous call for receiving unexpected messages using BMI.
 */
int bmi_comm_recvu(BMI_addr_t *peer_addr, void **recvbuf,
                   bmi_size_t *recvbuflen, bmi_msg_tag_t *tag) {
    int ret, outcount = 0;
    struct BMI_unexpected_info request_info;

    /* Wait for an initial request from client */
    do {
        ret = BMI_testunexpected(1, &outcount, &request_info, MAX_IDLE_TIME);
    } while (ret == 0 && outcount == 0);

    if (ret < 0) {
        fprintf(stderr, "bmi_comm_recvu: Request recv failure (bad state).\n");
        fprintf(stderr, "bmi_comm_recvu: BMI_testunexpected failed.\n");
        exit(1);
    }

    if (request_info.error_code != 0) {
        fprintf(stderr, "bmi_comm_recvu: Request recv failure (bad state).\n");
        exit(1);
    }

    *peer_addr = request_info.addr;
    *recvbuflen = request_info.size;
    *tag = request_info.tag;

    *recvbuf = BMI_memalloc(*peer_addr, *recvbuflen, BMI_RECV);
    if (!recvbuf) {
        fprintf(stderr, "bmi_comm_recvu: BMI_memalloc() failed.\n");
        exit(1);
    }

    memcpy(*recvbuf, request_info.buffer, *recvbuflen);

    BMI_unexpected_free(*peer_addr, request_info.buffer);
    return 0;
}

/*
 * bmi_comm_send
 * Synchronous call for sending multiple messages using BMI.
 */
int bmi_comm_send_list(BMI_addr_t peer_addr, size_t list_count,
                       const void *const *buffers, const bmi_size_t *buflens,
                       bmi_msg_tag_t tag, bmi_context_id context, bmi_size_t total_size) {
    bmi_op_id_t op_id;
    int ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    /* Post the BMI send requests and wait for its completion */
    ret = BMI_post_send_list(&op_id, peer_addr, buffers, buflens,
                             list_count, total_size, BMI_PRE_ALLOC,
                             tag, NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_send_list: BMI_post_send() failed.\n");
        exit(1);
    } else if (ret == 0) {
        do {
            ret = BMI_test(op_id, &outcount, &error_code, &actual_size, NULL,
                           MAX_IDLE_TIME, context);
        } while (ret == 0 && outcount == 0);

        if (ret < 0 || error_code != 0) {
            fprintf(stderr, "bmi_comm_send_list: Data send failed. %d\n", error_code);
            exit(1);
        }

        if (actual_size != total_size) {
            fprintf(stderr, "bmi_comm_send_list: Expected %ld but received %lu\n",
                    total_size, actual_size);
            exit(1);
        }
    }

    return 0;
}


/*
 * bmi_comm_recv_list
 * Synchronous call for receiving multiple messages using BMI.
 */
int bmi_comm_recv_list(BMI_addr_t peer_addr, size_t list_count,
                       void *const * buffers, const bmi_size_t *buflens,
                       bmi_msg_tag_t tag, bmi_context_id context, bmi_size_t total_size)
{
    bmi_op_id_t op_id;
    int ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    /* Post the BMI recv request and wait for its completion */
    ret = BMI_post_recv_list(&op_id, peer_addr, buffers, buflens, list_count,
                             total_size, &actual_size, BMI_PRE_ALLOC,
                             tag, NULL, context, NULL);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_recv_list: BMI_post_recv_list() failed.\n");
        exit(1);
    } else if (ret == 0) {
        do {
            ret = BMI_test(op_id, &outcount, &error_code, &actual_size, NULL,
                           MAX_IDLE_TIME, context);
        } while (ret == 0 && outcount == 0);

        if (ret < 0 || error_code != 0) {
            fprintf(stderr, "bmi_comm_recv_list: Data receive failed.\n");
            exit(1);
        }
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
