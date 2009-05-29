#ifndef IOFWD_ZOIDFSPOSIX_NAMECACHE_H
#define IOFWD_ZOIDFSPOSIX_NAMECACHE_H

#include "zoidfs.h"

/*
 * Maintains a local cache of pathname -> zoidfshandle
 * and zoidfshandle -> pathname
 *
 * Older entries are automatically removed if the capacity would be exceeded.
 *
 * This code is also used internally to implement the memory persistance
 * layer.
 */

int namecache_init (int capacity);

int namecache_destroy (); 

/* lookup; return non-zero if found */ 
int namecache_lookup_handle (const char * name, zoidfs_handle_t * handle);

int namecache_lookup_path (const zoidfs_handle_t * handle, char name, int
      bufsize); 

/* remove entry from the namecache; return non-zero if removed  */ 
int namecache_invalidate_handle (const zoidfs_handle_t * handle); 

int namecache_invalidate_path (const char * name); 

/* add entry */ 
int namecache_add (const char * name, const zoidfs_handle_t * handle); 

#endif
