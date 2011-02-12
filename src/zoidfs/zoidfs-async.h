#ifndef SRC_ZOIDFS_ZOIDFS_ASYNC_H
#define SRC_ZOIDFS_ZOIDFS_ASYNC_H

#include <limits.h>

/* TODO add C++ externs */

/* max timeout */
#define ZOIDFS_TIMEOUT_INF UINT_MAX
#define ZOIDFS_TIMEOUT_NOWAIT 0 

/* TODO add assert for TIMEOUT size in init functoion */

/* timeout type */
typedef unsigned int zoidfs_timeout_t;
enum zoidfs_comp_mask_t
{
    LOCAL=0x01, /* buffers are usable at client */
    REMOTE=0x02, /* remote operation done... buffers free on server */
    DONE=0x02, /* atm, alias remote */ /* XXX option in the lib to change what DONE
                                          means */
    /* other completion modes */
};

struct zoidfs_request_obj;

typedef (struct zoidfs_request_obj *) zoidfs_request_t;

int zoidfs_request_free(zoidfs_request_t * request);
int zoidfs_request_get_error(int * error);
int zoidfs_request_get_comp_state(zoidfs_comp_mask_t * state);

/* needs to be smart... only one call can drive state machines... others sleep
 * */
int zoidfs_test(zoidfs_request_t * request, zoidfs_timeout_t timeout,
        zoidfs_comp_mask_t mask);

int zoidfs_igetattr(zoidfs_request_t * request, /* in:ptr */
        const zoidfs_handle_t * handle /*in:ptr */,
        zoidfs_attr_t * attr /* inout:ptr */,
        zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */);
#endif
