#include <assert.h>
#include <string.h>

/* get access to HAVE_DISP... defines */
#include "iofwd_config.h"


#include "zint-handler.h"

#ifdef HAVE_DISPATCHER_PVFS2
#include "pvfs/zoidfs-pvfs2.h"
#endif

#ifdef HAVE_DISPATCHER_POSIX
#include "posix/zoidfs-posix.h"
#endif

#ifdef HAVE_DISPATCHER_LIBSYSIO
#include "sysio/zoidfs-sysio.h"
#endif

#ifdef HAVE_DISPATCHER_POSIX2
#include "posix2/zoidfs-posix2.h"
#endif

#include "local/zoidfs-local.h"


/**
 * NOTE: local should always be last as it will resolve paths successfully
 * as long as the path exists (even if its a mountpoint for a different fs).
 */

static zint_handler_t * zint_handlers[] =
{
#ifdef HAVE_DISPATCHER_PVFS2
    &pvfs2_handler,
#endif

/*
 * If libsysio is available, don't build posix.
 * TODO: This needs to be improved. Should they
 * both be built or only one or the other?
 */
#ifdef HAVE_DISPATCHER_LIBSYSIO
    &sysio_handler,
#endif /* HAVE_DISPATCHER_LIBSYSIO */
#ifdef HAVE_DISPATCHER_POSIX2
    &posix2_handler,
#endif
#ifdef HAVE_DISPATCHER_POSIX
    &posix_handler,
#endif /* HAVE_DISPATCHER_POSIX */
    &local_handler
};

#define ZINT_HANDLERS_COUNT ((int) (sizeof(zint_handlers) / sizeof(zint_handlers[0])))


zint_handler_t * zint_get_handler(zint_handle_type_t type)
{
    assert(type < ZINT_HANDLERS_COUNT);
    return zint_handlers[(uint32_t)(type)];
}

int zoidfs_dispatch_handle_eq (const zoidfs_handle_t * h1,
      const zoidfs_handle_t * h2)
{
   /* skip first 4 bytes */
   return memcmp (&h1->data[4], &h2->data[4], sizeof(h2->data)-4)==0;
}

zint_handler_t * zint_get_handler_from_path(
    const char * path,
    char * newpath,
    int newpath_maxlen,
    int * id,
    zoidfs_handle_t * newhandle,
    int * usenew)
{
    int ret;
    int i = 0;

    for(; i < ZINT_HANDLERS_COUNT; ++i)
    {
        ret = zint_handlers[i]->resolve_path(
            path, newpath, newpath_maxlen, newhandle,
            usenew);
        if(ret == ZFS_OK)
        {
           if (id)
              *id = i;
           return zint_handlers[i];
        }
    }
    return NULL;
}

int zint_locate_handler_handle(const zoidfs_handle_t * handle,
      zint_handler_t ** handler)
{
   *handler = zint_get_handler(ZOIDFS_HANDLE_TYPE(handle));
   if(!*handler)
   {
      return -1;
   }
   return ZOIDFS_HANDLE_TYPE(handle);
}

int zint_locate_handler_path(const char * path,
                        zint_handler_t ** handler,
                        char * newpath,
                        int newpath_maxlen,
                        zoidfs_handle_t * newhandle,
                        int * usenew)
{
   int id;
   *handler = zint_get_handler_from_path(
         path, newpath, newpath_maxlen, &id, newhandle,
         usenew);
   if(!*handler)
   {
      return -1;
   }
   return id;
}

int zint_handler_count ()
{
   return ZINT_HANDLERS_COUNT;
}


int zint_ping_handlers ()
{
    int ret;
    int i;

    for(i=0; i < zint_handler_count();  ++i)
    {
        ret = zint_handlers[i]->null();
        if(ret != ZFS_OK)
           return ret;
    }
    return ZFS_OK;
}

int zint_finalize_handlers ()
{
    int ret;
    int i;
    int err = ZFS_OK;

    for(i=0; i < zint_handler_count();  ++i)
    {
        ret = zint_handlers[i]->finalize();
        /* Continue shutting down other handlers even if an error occurs */
        if(ret != ZFS_OK)
            err = ret ;
    }
    return err;
}

int zint_initialize_handlers ()
{
    int ret;
    int i = 0;

    for(; i < ZINT_HANDLERS_COUNT; ++i)
    {
        ret = zint_handlers[i]->init();
        if(ret != ZFS_OK)
        {
            return ret;
        }
    }
    return ZFS_OK;
}
