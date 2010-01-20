#include <assert.h>
#include <string.h>
#include <ctype.h>

/* get access to HAVE_DISP... defines */
#include "iofwd_config.h"


#include "zoidfs/dispatcher/zint-handler.h"

#ifdef USE_DISPATCHER_NOFS
#include "zoidfs/dispatcher/nofs/zoidfs-nofs.h"
#endif /* USE_DISPATCHER_NOFS */

#ifdef HAVE_DISPATCHER_PVFS2
#include "zoidfs/dispatcher/pvfs/zoidfs-pvfs2.h"
#endif /* HAVE_DISPATCHER_PVFS2 */

#ifdef HAVE_DISPATCHER_LIBSYSIO
#include "zoidfs/dispatcher/sysio/zoidfs-sysio.h"
#endif /* HAVE_DISPATCHER_LIBSYSIO */

#ifdef HAVE_DISPATCHER_POSIX
#include "zoidfs/dispatcher/posix/zoidfs-posix.h"
#endif /* HAVE_DISPATCHER_POSIX */


#include "zoidfs/dispatcher/local/zoidfs-local.h"

/**
 * NOTE: local should always be last as it will resolve paths successfully
 * as long as the path exists (even if its a mountpoint for a different fs).
 */

static zint_handler_t * zint_server_handlers_values[] =
{
#ifdef USE_DISPATCHER_NOFS
    &nofs_handler,
#endif /*  USE_DISPATCHER_NOFS */
#ifdef HAVE_DISPATCHER_PVFS2
    &pvfs2_handler,
#endif /* HAVE_DISPATCHER_PVFS2 */
#ifdef HAVE_DISPATCHER_LIBSYSIO
    &sysio_handler,
#endif /* HAVE_DISPATCHER_LIBSYSIO */
#ifdef HAVE_DISPATCHER_POSIX
    &posix_handler,
#endif /* HAVE_DISPATCHER_POSIX */
    &local_handler
};

static char * zint_server_handlers_keys[] =
{
#ifdef USE_DISPATCHER_NOFS
    "nofs",
#endif /*  USE_DISPATCHER_NOFS */
#ifdef HAVE_DISPATCHER_PVFS2
    "pvfs2",
#endif /* HAVE_DISPATCHER_PVFS2 */
#ifdef HAVE_DISPATCHER_LIBSYSIO
    "sysio",
#endif /* HAVE_DISPATCHER_LIBSYSIO */
#ifdef HAVE_DISPATCHER_POSIX
    "posix",
#endif /* HAVE_DISPATCHER_POSIX */
    "local"
};

/* total number of handlers compiled into the server */
#define ZINT_SERVER_HANDLERS_COUNT ((unsigned int) (sizeof(zint_server_handlers_values) / sizeof(zint_server_handlers_values[0])))
#define ZINT_SERVER_HANDLERS_LOCAL_INDEX (ZINT_SERVER_HANDLERS_COUNT - 1)

/* server handlers to be used by the user */
static unsigned int ZINT_HANDLERS_COUNT = 0;
static zint_handler_t ** zint_handlers = NULL;
static char ** zint_handlers_keys = NULL;

/* convert the string to lower case */
static int zint_str_tolower(char * str)
{
    unsigned int i = 0;

    for( i = 0 ; i < strlen(str) ; i++)
    {
        str[i] = tolower(str[i]);
    }

    return ZFS_OK;
}

/* setup the server handlers based on the user handlers */
int zint_setup_handlers(int n, char * user_handlers[])
{
    int i = 0;
    unsigned int j = 0;
    int hsize = 0;

    /* convert user handler requests to upper case */
    /* was the local handler specified */
    for( i = 0 ; i < n ; i++)
    {
        zint_str_tolower(user_handlers[i]);
        if(strcmp(user_handlers[i], zint_server_handlers_keys[ZINT_SERVER_HANDLERS_LOCAL_INDEX]) != 0)
        {
           hsize++;
        }
    }

    /* allocate the handler array */
    zint_handlers = (zint_handler_t **)malloc(sizeof(zint_handler_t) * (hsize + 1));
    zint_handlers_keys = (char **)malloc(sizeof(char *) * (hsize + 1));

    /* insert the handlers to be used by the server */

    /* for each user handler */
    for( i = 0 ; i < n ; i++)
    {
        /* for each server handler */
        for( j = 0 ; j < ZINT_SERVER_HANDLERS_COUNT ; j++)
        {
            /* if the user requested this server handler, add it to the list */
            /* if the requested handler was local, ignore since it is forced to be the last handle */
            if(strcmp(zint_server_handlers_keys[j], user_handlers[i]) == 0 &&
                strcmp(zint_server_handlers_keys[ZINT_SERVER_HANDLERS_LOCAL_INDEX], user_handlers[i]) != 0)
            {
                zint_handlers[ZINT_HANDLERS_COUNT] = zint_server_handlers_values[j];
                zint_handlers_keys[ZINT_HANDLERS_COUNT] = zint_server_handlers_keys[j];
                ZINT_HANDLERS_COUNT++;
            }
        }
    }

    /* always set the last handler as local */
    zint_handlers[ZINT_HANDLERS_COUNT] = zint_server_handlers_values[ZINT_SERVER_HANDLERS_LOCAL_INDEX];
    zint_handlers_keys[ZINT_HANDLERS_COUNT] = zint_server_handlers_keys[ZINT_SERVER_HANDLERS_LOCAL_INDEX];
    ZINT_HANDLERS_COUNT++;

    return ZFS_OK;
}

zint_handler_t * zint_get_handler(zint_handle_type_t type)
{
    /* make sure that the handlers were setup */
    if(ZINT_HANDLERS_COUNT == 0)
        return NULL;

    if (type >= ZINT_HANDLERS_COUNT)
        return NULL;

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
    unsigned int i = 0;

    /* make sure that the handlers were setup */
    if(ZINT_HANDLERS_COUNT == 0)
        return NULL;

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
    /* make sure that the handlers were setup */
    if(ZINT_HANDLERS_COUNT == 0)
        return -1;

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

    /* make sure that the handlers were setup */
    if(ZINT_HANDLERS_COUNT == 0)
        return -1;

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

    /* cleanup zint_handlers */
    free(zint_handlers);
    free(zint_handlers_keys);
    zint_handlers = NULL;
    zint_handlers_keys = NULL;

    return err;
}

int zint_set_handler_options(ConfigHandle c, SectionHandle s)
{
    int ret;
    unsigned int i = 0;

    for(i = 0 ; i < ZINT_HANDLERS_COUNT; ++i)
    {
        SectionHandle cur_handle;
        ret = cf_openSection(c, s, zint_handlers_keys[i], &cur_handle);
        if(ret < 0)
        {
            return ZFSERR_OTHER;
        }

        ret = zint_handlers[i]->set_options(c, cur_handle);
        if(ret != ZFS_OK)
        {
            return ret;
        }
    }

    return ZFS_OK;
}

int zint_initialize_handlers()
{
    int ret;
    unsigned int i = 0;

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
