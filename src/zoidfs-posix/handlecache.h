#ifndef ZOIDFS_POSIX_HANDLECACHE_H
#define ZOIDFS_POSIX_HANDLECACHE_H

#include <stdio.h>

/**
 * Make sure to return the same file handle 
 * every time the file is opened.
 * Automatically close / reopen files when needed.
 */

typedef void *  hc_value_t; 

enum { HC_READ = 0x01, HC_WRITE = 0x02; };

typedef int (*handlecache_open_t) (const char * path, hc_value_t * dst, int
      flags, bool upgrade);

typedef int (*handlecache_close_t) (hc_value_t * dst); 

int handlecache_init (int size, handlecache_open_t openfunc, 
      handlecache_close_t closefunc);

int handlecache_destroy (); 

hc_value_t * handlecache_get (const char * path);


#endif
