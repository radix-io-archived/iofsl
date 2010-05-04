#ifndef ZOIDFS_UTIL_H
#define ZOIDFS_UTIL_H

#include <string.h>
#include "zoidfs/zoidfs.h"

#ifdef __cplusplus
namespace zoidfs
{
   extern "C"
   {
#endif

/* Convert zoidfs handle into human readable string */ 
int zoidfs_handle_to_text (const zoidfs_handle_t * handle, char * buf, int
      bufsize); 

/* Convert human readable zoidfs handle into handle */ 
int zoidfs_text_to_handle (const char * buf, zoidfs_handle_t * handle); 

/* Return pointer to string repr of handle */ 
const char * zoidfs_handle_string (const zoidfs_handle_t * handle);


/* Return 1 if the two handles are equal */
static inline int zoidfs_handle_equal (const zoidfs_handle_t * h1, const
      zoidfs_handle_t * h2)
{
   return (memcmp (h1, h2, sizeof(zoidfs_handle_t))==0); 
}

#ifdef __cplusplus
   }
}
#endif


#endif
