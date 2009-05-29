#ifndef ZOIDFS_POSIX_PERSIST_CLEAN_H
#define ZOIDFS_POSIX_PERSIST_CLEAN_H

#include "../zint-handler.h"
#include "persist.h"

/**
 * Persist wrappers that mask out the reserved bytes in the zoidfs handle
 */


static inline int persist_clean_add (persist_op_t * con, const char * name,
      const zoidfs_handle_t * handle)
{
   zoidfs_handle_t newhandle;
   ZINT_COPY_CLEANUP_HANDLE (handle, &newhandle); 
   return persist_add (con, name, &newhandle); 
}

static inline int persist_clean_rename (persist_op_t * con, const char * f1, const
      char * f2, int dir)
{
   return persist_rename (con, f1, f2, dir); 
}

static inline int persist_clean_readdir (persist_op_t * con, const char * dir, persist_filler_t filler, 
      void * fillerdata)
{
   return persist_readdir (con, dir, filler, fillerdata); 
}


static inline int persist_clean_handle_to_filename (persist_op_t * con,
      const zoidfs_handle_t * handle,
      char * buf, unsigned int bufsize)
{
   zoidfs_handle_t newhandle;
   ZINT_COPY_CLEANUP_HANDLE(handle, &newhandle); 
   return persist_handle_to_filename (con, &newhandle, buf, bufsize); 
}

static inline int persist_clean_filename_to_handle (persist_op_t * con, const
      char * filename, zoidfs_handle_t * handle, int autoadd)
{
   /* TODO: add namecache here */ 
   return persist_filename_to_handle (con, filename, handle, autoadd); 
}


static inline int persist_clean_purge (persist_op_t * con, const char * filename,
      int prefix)
{
   return persist_purge (con, filename, prefix); 
}



#endif
