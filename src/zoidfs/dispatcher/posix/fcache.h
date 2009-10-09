#ifndef POSIX_FCACHE_H
#define POSIX_FCACHE_H

#include "zoidfs/zoidfs.h"

/* ======================== Filename cache ================================ */

struct fcache_instance;
typedef struct fcache_instance * fcache_handle; 

fcache_handle filename_create (int cachesize); 

int filename_lookup (fcache_handle, const zoidfs_handle_t * h, char * buf, int bufsize);

/* if handle exists, replace entry */ 
int filename_add    (fcache_handle, const zoidfs_handle_t * h, const char * buf); 

int filename_remove (fcache_handle, const zoidfs_handle_t * h); 

int filename_destroy (fcache_handle); 


#endif
