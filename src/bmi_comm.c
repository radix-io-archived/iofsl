/*
 * bmi_comm.c
 * Synchronous BMI calls. BMI communication is inherently asynchronous. We use
 * wrapper functions to create synchronous send/recv calls.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#include "bmi_comm.h"

/* Synchronous call for sending messages using BMI */
int32_t bmi_comm_send(BMI_addr_t peer_addr, void *buffer, bmi_size_t buflen,
                      bmi_msg_tag_t tag, bmi_context_id context) {
    bmi_op_id_t op_id;
    int32_t ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    ret = BMI_post_send(&op_id, peer_addr, buffer, buflen, BMI_PRE_ALLOC, tag,
                        NULL, context);
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


/* Synchronous call for receiving messages using BMI */
int32_t bmi_comm_recv(BMI_addr_t peer_addr, void *buffer, bmi_size_t buflen,
                      bmi_msg_tag_t tag, bmi_context_id context) {
    bmi_op_id_t op_id;
    int32_t ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    ret = BMI_post_recv(&op_id, peer_addr, buffer, buflen, &actual_size,
                        BMI_PRE_ALLOC, tag, NULL, context);
    if (ret < 0) {
        fprintf(stderr, "bmi_comm_recv: BMI_post_recv() failed.\n");
        exit(1);
    } else if (ret == 0) {
        do {
            ret = BMI_test(op_id, &outcount, &error_code, &actual_size, NULL,
                           MAX_IDLE_TIME, context);
        } while (ret == 0 && outcount == 0);

        if (ret < 0 || error_code != 0) {
            fprintf(stderr, "bmi_comm_recv: Data receive failed.\n");
            exit(1);
        }
    }

    return 0;
}


/* Synchronous call for sending unexpected messages using BMI */
int32_t bmi_comm_sendu(BMI_addr_t peer_addr, void *buffer, bmi_size_t buflen,
                       bmi_msg_tag_t tag, bmi_context_id context) {
    bmi_op_id_t op_id;
    int32_t ret, outcount;
    bmi_size_t actual_size;
    bmi_error_code_t error_code;

    ret = BMI_post_sendunexpected(&op_id, peer_addr, buffer, buflen,
                                  BMI_PRE_ALLOC, tag, NULL, context);
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

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
