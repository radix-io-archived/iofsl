#include "zoidfs/zoidfs.h"

struct IOFWDClient;

void * IOFWDClient_cwrapper_allocate(char * address, char * configfile);
void IOFWDClient_cwrapper_free(void * ptr);

int IOFWDClient_cwrapper_init(struct IOFWDClient *, zoidfs_op_hint_t *);
int IOFWDClient_cwrapper_finalize(struct IOFWDClient *, zoidfs_op_hint_t *);
int IOFWDClient_cwrapper_create(struct IOFWDClient *, const zoidfs_handle_t *, const char *,
        const char *, const zoidfs_sattr_t *, zoidfs_handle_t *, int *,
        zoidfs_op_hint_t *);
int IOFWDClient_cwrapper_remove(struct IOFWDClient *, const zoidfs_handle_t *, const char *,
        const char *, zoidfs_cache_hint_t *, zoidfs_op_hint_t *);
int IOFWDClient_cwrapper_lookup(struct IOFWDClient *, const zoidfs_handle_t *, const char *,
        const char *, zoidfs_handle_t *, zoidfs_op_hint_t *);

static struct IOFWDClient * iofwdclient_ptr = NULL;

int zoidfs_init(void)
{
    char * address = NULL;
    char * configfile = NULL;

    if(iofwdclient_ptr == NULL)
        iofwdclient_ptr = IOFWDClient_cwrapper_allocate(address, configfile);

    return IOFWDClient_cwrapper_init(iofwdclient_ptr, NULL);
}

int zoidfs_finalize(void)
{
    int ret = IOFWDClient_cwrapper_finalize(iofwdclient_ptr, NULL);

    if(iofwdclient_ptr)
        IOFWDClient_cwrapper_free(iofwdclient_ptr);

    return ret;
}

int zoidfs_create(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  const zoidfs_sattr_t * attr /* in:ptr */,
                  zoidfs_handle_t * handle /* out:ptr */,
                  int * created /* out:ptr:nullok */,
                  zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */)
{
    return IOFWDClient_cwrapper_create(iofwdclient_ptr, parent_handle,
            component_name, full_path, attr, handle, created, op_hint);
}

int zoidfs_remove(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  zoidfs_cache_hint_t * parent_hint /* out:ptr:nullok */, 
                  zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */)
{
    return IOFWDClient_cwrapper_remove(iofwdclient_ptr, parent_handle,
            component_name, full_path, parent_hint, op_hint);
}

int zoidfs_lookup(const zoidfs_handle_t * parent_handle /* in:ptr:nullok */,
                  const char * component_name /* in:str:nullok */,
                  const char * full_path /* in:str:nullok */,
                  zoidfs_handle_t * handle /* out:ptr */,
                  zoidfs_op_hint_t * op_hint /* inout:ptr:nullok */)
{
    return IOFWDClient_cwrapper_lookup(iofwdclient_ptr, parent_handle,
            component_name, full_path, handle, op_hint);
}
