#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "fcache.h"
#include "c-util/gencache.h"

struct fcache_instance
{
   gencache_handle gencache; 
}; 

typedef struct fcache_instance fcache_instance; 

static int filename_gencache_item_free (void * item)
{
   /* item is a string */
   free ( (char *) item); 
   return 1; 
}

static int filename_gencache_key_free (void * item)
{
   /* key is a zoidfshandle */
   free ( (zoidfs_handle_t *) item); 
   return 1; 
}

static int filename_gencache_key_compare (const void * i1, const void * i2)
{
   /* keys are uint8_t arrays */
   /* compare function returns 1 if equal, 0 if non-equal */
   return (memcmp (((zoidfs_handle_t *) i1)->data, ((zoidfs_handle_t *)i2)->data, 
         sizeof (((zoidfs_handle_t *)i1)->data)) == 0 ? 1 : 0); 
}

static unsigned long filename_gencache_key_hash (gencache_key_t key)
{
   const zoidfs_handle_t * h = (zoidfs_handle_t *) key; 
   const unsigned char * ptr = (unsigned char *) &h->data; 
   unsigned char init[sizeof(unsigned long)];
   unsigned int  i; 
   unsigned int pos = 0; 

   memset (&init, 0, sizeof(init)); 

   for (i=0; i<sizeof (h->data); ++i)
   {
      init[pos++] ^= *ptr++; 
      if (pos == sizeof(unsigned long))
         pos = 0; 
   }
   return *(unsigned long *) &init[0]; 
}


fcache_handle filename_create (int cachesize)
{
   gencache_init_t init; 
   fcache_handle h;

   init.max_cache_size = cachesize; 
   init.fn_item_free = filename_gencache_item_free; 
   init.fn_key_free = filename_gencache_key_free; 
   init.fn_key_compare = filename_gencache_key_compare;
   init.fn_key_hash = filename_gencache_key_hash; 
   
   h = (fcache_handle) malloc (sizeof (fcache_instance)); 
   h->gencache = gencache_init (&init); 
   return h; 
}

int filename_destroy (fcache_handle handle)
{
   int ret = gencache_done (handle->gencache); 
   free ((fcache_instance *) handle); 
   return ret; 
}

int filename_lookup (fcache_handle fh, const zoidfs_handle_t * h, char * buf, int bufsize)
{
   const char * data; 
   gencache_lock_info lock; 

   /* need to lock cache entry, otherwise some other thread could free the
    * string while we're copying it */ 
   int ret = gencache_key_lookup (fh->gencache, (gencache_key_t) h, (gencache_value_t *) &data, &lock); 
   if (ret)
   {
      strncpy (buf, data, bufsize-1); 
      buf[bufsize-1] = 0; 
      gencache_key_unlock (fh->gencache, &lock); 
   }

   return ret; 
}

/*
 * Makes a copy of the data */ 
int filename_add    (fcache_handle fh, const zoidfs_handle_t * h, const char * buf)
{
   char * dup = strdup (buf); 

   zoidfs_handle_t * myhandle = malloc (sizeof (zoidfs_handle_t)); 

   *myhandle = *h; 

   /* no need to lock the entry */ 
   int ret = gencache_key_add (fh->gencache, (gencache_key_t) myhandle, dup, 0); 

   return ret; 
}

int filename_remove (fcache_handle fh, const zoidfs_handle_t * h)
{
   return gencache_key_remove (fh->gencache, (gencache_key_t) h); 
}


