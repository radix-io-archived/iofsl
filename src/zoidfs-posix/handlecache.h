#ifndef ZOIDFS_POSIX_HANDLECACHE_H
#define ZOIDFS_POSIX_HANDLECACHE_H

#include <stdio.h>
#include "iofwdbool.h"

/**
 * Make sure to return the same file handle 
 * every time the file is opened.
 * Automatically close / reopen files when needed.
 */


/* The file handle type of the client */
typedef FILE *  hc_value_t; 

/* Our handle type */
typedef void * hc_item_handle_t; 

enum { HC_READ = 0x01, HC_WRITE = 0x02 };

typedef int (*handlecache_open_t) (const char * path, hc_value_t * dst, int
      flags, iofwdbool_t upgrade);

typedef int (*handlecache_close_t) (hc_value_t * dst); 

int handlecache_init (int size, handlecache_open_t openfunc, 
      handlecache_close_t closefunc);

int handlecache_destroy (); 

/* Lookup a handle; store in handle; return user handle associated with file */ 
int handlecache_get (const char * path, int flags, hc_item_handle_t * handle,
      hc_value_t * user);

/* Make sure the handle is valid; return user handle associated with file */ 
int handlecache_validate (const hc_item_handle_t * handle, hc_value_t * user); 

#endif
