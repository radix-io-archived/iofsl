#include "iofwd-ion-net.hh"
#include "src/zoidfs/zoidfs.h"

/* ifuncs. timeout define, ... */
#include "src/zoidfs/zoidfs-async.h"

void iofwd_client_init()
{
}

/* exception to error code conversion ... stuff err info */
static int zoidfs_exception_to_error(const std::exception & e)
{
}

int zoidfs_getattr(const zoidfs_handle_t * handle /* in:ptr */,
                   zoidfs_attr_t * attr /* inout:ptr */,
                   zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */)
{
        zoidfs_request_t request;

        zoidfs_igetattr(&request, handle, attr, op_hint);

        zoidfs_test(&request, ZOIDFS_TIMEOUT_INF);
}

int zoidfs_igetattr(zoidfs_request_t * request, const zoidfs_handle_t * handle /* in:ptr */,
                   zoidfs_attr_t * attr /* inout:ptr */,
                   zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */)
{
        try
        {
            /* allocate the request */

            /* bind the request */
        }
        catch(...)
        {
        }
}

/* update the request and call the cv */
static void update_comp(zoidfs_request_t request,
        zoidfs_comp_mask_t compstate,
        const CBException & e)
{
    /* check if e has a valid exception */
    if()
    {
        /* handle the error, trickle the error back to the client */
    }
    /* update the request and signal the comp variable */
    else
    {
    }
}

int zoidfs_cb_igetattr(const IOFWDClientCB & cb,
        int * ret, /*out:ptr*/
        const zoidfs_handle_t * handle /* in:ptr */,
        zoidfs_attr_t * attr /* inout:ptr */,
        zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */)
{
    /* create SM (pass CB, getattr params) */

    /* submit to SM Manager */
}

int zoidfs_test(zoidfs_request_t * request, int timeout=0)
{
}
