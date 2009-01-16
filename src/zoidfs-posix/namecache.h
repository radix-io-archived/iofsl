#ifndef IOFWD_ZOIDFSPOSIX_NAMECACHE_H
#define IOFWD_ZOIDFSPOSIX_NAMECACHE_H

#include "zoidfs.h"

/*
 * Maintains a local cache of pathname -> zoidfshandle
 */

int namecache_init ();

int namecache_destroy (); 

int namecache_lookup (const char * name, zoidfs_handle_t * handle);

int namecache_invalidate (const char * name); 

int namecache_add (const char * name, const zoidfs_handle_t * handle, int
      exclusive); 

#endif
