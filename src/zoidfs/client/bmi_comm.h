/*
 * bmi_comm.h
 * Synchronous BMI calls. BMI communication is inherently asynchronous. We use
 * wrapper functions to create synchronous send/recv calls.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 * Jason Cope <copej@mcs.anl.gov>
 */

#ifndef _BMI_COMM_H_
#define _BMI_COMM_H_

#include<stdio.h>
#include<errno.h>
#include<string.h>
#include<unistd.h>

#include "bmi.h"

#define MAX_IDLE_TIME 10
#define ION_ENV "ZOIDFS_ION_NAME"

/* blocking ZoidFS BMI wrappers */
int bmi_comm_send(BMI_addr_t, const void *, bmi_size_t, bmi_msg_tag_t,
                  bmi_context_id);
int bmi_comm_recv(BMI_addr_t, void *, bmi_size_t, bmi_msg_tag_t,
                  bmi_context_id, bmi_size_t * actual_size);
int bmi_comm_sendu(BMI_addr_t, const void *, bmi_size_t, bmi_msg_tag_t,
                   bmi_context_id);
int bmi_comm_recvu(BMI_addr_t *, void **, bmi_size_t *, bmi_msg_tag_t *);
int bmi_comm_send_list(BMI_addr_t,
                       size_t, const void *const *buffer_list,
                       const bmi_size_t *, bmi_msg_tag_t, bmi_context_id, bmi_size_t total_size);
int bmi_comm_recv_list(BMI_addr_t,
                       size_t, void *const *buffer_list, const bmi_size_t *,
                       bmi_msg_tag_t, bmi_context_id, bmi_size_t total_size);

/* nonblocking ZoidFS BMI wrappers */
int bmi_comm_isend(BMI_addr_t peer_addr, const void *buffer, bmi_size_t buflen,
                  bmi_msg_tag_t tag, bmi_context_id context, bmi_op_id_t * op_id);
int bmi_comm_isend_wait(bmi_op_id_t op_id, bmi_size_t buflen, bmi_context_id context);
int bmi_comm_irecv(BMI_addr_t peer_addr, void *buffer, bmi_size_t buflen,
                  bmi_msg_tag_t tag, bmi_context_id context, bmi_size_t * actual_size, bmi_op_id_t * op_id);
int bmi_comm_irecv_wait(bmi_op_id_t op_id, bmi_size_t * actual_size, bmi_context_id context);
int bmi_comm_isendu(BMI_addr_t peer_addr, const void *buffer, bmi_size_t buflen,
                   bmi_msg_tag_t tag, bmi_context_id context, bmi_op_id_t * op_id);
int bmi_comm_isendu_wait(bmi_size_t buflen, bmi_context_id context, bmi_op_id_t op_id);
int bmi_comm_isend_list(BMI_addr_t peer_addr, size_t list_count, const void *const *buffers, const bmi_size_t *buflens, bmi_msg_tag_t tag, bmi_context_id context, bmi_size_t total_size, bmi_op_id_t * op_id);
int bmi_comm_isend_list_wait(bmi_op_id_t op_id, bmi_context_id context, bmi_size_t total_size);
int bmi_comm_irecv_list(BMI_addr_t peer_addr, size_t list_count, void *const * buffers, const bmi_size_t *buflens, bmi_msg_tag_t tag, bmi_context_id context, bmi_size_t total_size, bmi_size_t * actual_size, bmi_op_id_t * op_id);
int bmi_comm_irecv_list_wait(bmi_op_id_t op_id, bmi_context_id context, bmi_size_t * actual_size);
#endif /* _BMI_COMM_H_ */

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
