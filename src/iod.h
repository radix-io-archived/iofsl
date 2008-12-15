/*
 * iod.h
 * I/O daemon. The CNs communicate with the IONs over BMI.
 *
 * Nawab Ali <alin@cse.ohio-state.edu>
 */

#ifndef _IOD_H_
#define _IOD_H_

#include "bmi.h"
#include "bmi_comm.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#define BUFSIZE 1024
#define MAX_IDLE_TIME 10

int32_t iod_bmi_init(char *);
int32_t iod_bmi_finalize(void);
int32_t iod_bmi_handshake(BMI_addr_t *, bmi_msg_tag_t, bmi_context_id context);

static int32_t iod_check_uri(char *);
static void iod_get_network(char *, char **);

#endif

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
