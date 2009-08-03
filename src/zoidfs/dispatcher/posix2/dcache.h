#ifndef POSIX2_DCACHE_H
#define POSIX2_DCACHE_H

#include "c-util/gencache.h"
#include "zoidfs.h"

typedef struct
{
   int fd; 
   const zoidfs_handle_t * handle; 
   gencache_lock_info lock; 
} Descriptor; 

struct dcache_instance;
typedef struct dcache_instance * dcache_handle; 

dcache_handle dcache_create (int cachesize); 
int dcache_destroy (dcache_handle); 

/* Try to obtain a cached filedescriptor for the given zoidfs handle;
 * Store the result in dest */
int dcache_getfd (dcache_handle, const zoidfs_handle_t * handle, Descriptor *
      dest); 

/* Add a file descriptor to the cache; If success, descriptor will be updated
 * (and needs to be released); Will fail if the handle is already
 * present */
int dcache_addfd (dcache_handle, const zoidfs_handle_t * handle, int fd,
      Descriptor * dest); 

/* Return a file descriptor to the cache */
int dcache_releasefd (dcache_handle, Descriptor * dest); 

/* Remove a descriptor from the cache; Blocks until FD is removed */
int dcache_removefd (dcache_handle, const zoidfs_handle_t * handle);

#endif
