#ifndef ZOIDFS_POSIX_HANDLECACHE_H
#define ZOIDFS_POSIX_HANDLECACHE_H

#include <stdio.h>
#include "zoidfs.h"

/**
 * Make sure to return the same file handle 
 * every time the file is opened.
 * Automatically close / reopen files when needed.
 */


/* The file handle type of file access functions */ 
typedef int  hc_item_value_t; 


/* Function that is called when we remove an entry */ 
typedef int (*handlecache_removefunc_t) (hc_item_value_t * value); 

int handlecache_init (int size, handlecache_removefunc_t remove); 

int handlecache_destroy (); 

/* Lookup entry in the cache:
 *   * if not found, return 0
 *   * if found, return 1 and move item to the end of the gc queue
 *
 *   (could even avoid the last step if the number of entries is << cache
 *   limit)
 */
int handlecache_lookup (const zoidfs_handle_t * key, hc_item_value_t * val);

/*
 * Add new entry, possibly removing an older entry 
 */
int handlecache_add (const zoidfs_handle_t * key, hc_item_value_t * val); 

/*
 * Remove entry from cache
 */
int handlecache_purge (const zoidfs_handle_t * key); 


#endif
