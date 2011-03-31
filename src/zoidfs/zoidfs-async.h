#ifndef ZOIDFS_ZOIDFS_ASYNC_H
#define ZOIDFS_ZOIDFS_ASYNC_H

#include "zoidfs.h"
#include <limits.h>

#ifdef __cplusplus
namespace zoidfs
{
   extern "C"
   {
#endif

/* timeout type */
typedef unsigned int zoidfs_timeout_t;

/* Timeout constants */
#define ZOIDFS_TIMEOUT_INF       UINT_MAX
#define ZOIDFS_TIMEOUT_NOWAIT    0U

enum zoidfs_comp_mask_t
{
    ZFS_COMP_NONE    = 0x0000,
    ZFS_COMP_LOCAL   = 0x0001, /* buffers are usable at client */
    ZFS_COMP_REMOTE  = 0x0002, /* remote operation completed */
    ZFS_COMP_ERROR   = 0x1000, /* --- private: do not use ---- */

    /* atm, alias remote */
    ZFS_COMP_DONE=ZFS_COMP_REMOTE
    /* other completion modes */

};

/* ========================================================================
 * ====== Request Functions ===============================================
 * ======================================================================== */

/* zoidfs_request_t is the size of a void pointer */
typedef void * zoidfs_request_t;

#define ZOIDFS_REQUEST_NULL (zoidfs_request_t) 0

/**
 * Free request. A request can be freed even if it didn't complete yet.
 * A request always needs to be freed, even if it was successfully completed
 * by one of the wait functions.
 *
 * Sets request to ZOIDFS_REQUEST_NULL
 */
int zoidfs_request_free(zoidfs_request_t * request);

/**
 * Return the return code for operation associated with request.
 * @TODO: Change name here? return/status instead of 'error'?
 */
int zoidfs_request_get_error(zoidfs_request_t request, int * error);

/**
 * Return the completion status of the specified request.
 */
int zoidfs_request_get_comp_state(zoidfs_request_t request,
                                  zoidfs_comp_mask_t * state);

/**
 * Wait at most timeout for the specified request to change status to one of
 * the status bits specified in mask.
 *
 * Timeout is in milliseconds.
 *
 * Returns ZFSERR_TIMEOUT if the test did not succeed in timeout milliseconds,
 * and ZFS_OK otherwise.
 * */
int zoidfs_request_test(zoidfs_request_t request, zoidfs_timeout_t timeout,
        zoidfs_comp_mask_t mask);

/* ========================================================================
 * ==== Async ZoidFS functions ============================================
 * ======================================================================== */

/* Background progress function.
 *    - minwait indicates the minimum amount of time this function needs
 *      to try to make progress. Unless there is an error, zoidfs_progress()
 *      will not return before minwait time elapses.
 *
 * In general (i.e. no error), this function will wait until:
 *    - minwait time elapses
 *    - At least one progress event occured
 *       -or- maxwait time elapses.
 *
 * (Possible todo: have function return an error condition if there is no work
 * pending)
 */
int zoidfs_progress (zoidfs_timeout_t minwait, zoidfs_timeout_t maxwait);

/* ========================================================================
 * ==== Async ZoidFS functions ============================================
 * ======================================================================== */

int zoidfs_igetattr(zoidfs_request_t * request, /* in:ptr */
        const zoidfs_handle_t * handle /*in:ptr */,
        zoidfs_attr_t * attr /* inout:ptr */,
        zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */);

#ifdef __cplusplus
    }  /* extern "C" */
} /* namespace zoidfs */
#endif

#endif
