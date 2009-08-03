#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "dcache.h"

/** NOTE: dcache has zoidfs_handle_t * as key, FDValue * as data */ 

struct dcache_instance
{
   gencache_handle gencache; 
   pthread_mutex_t lock; 
}; 

/* the value type for the gencache */
typedef struct 
{
   int fd;
/*   pthread_mutex_t lock;  */
} FDValue; 

typedef struct dcache_instance dcache_instance; 


/* forward */
/* static int getfd (const char * filename, int * err); */ 

/* ======================================================================== */

static int dcache_item_free (void * item)
{
   const FDValue * v = (const FDValue *) item; 
   if (close (v->fd) < 0)
   {
      perror ("Error closing fd when removing from cache:"); 
   }

   /* pthread_mutex_destroy (&v->lock); */

   free ((FDValue *) v); 

   return 1; 
}

static int dcache_key_free (void * item)
{
   free ((zoidfs_handle_t*) item); 
   return 1; 
}

static int dcache_key_compare (const void * i1, const void * i2)
{
   /* keys are uint8_t arrays */
   /* compare function returns 1 if equal, 0 if non-equal */
   /* @TODO: convert this to a zoidfs_handle_cmp function */
   return (memcmp (((zoidfs_handle_t *) i1)->data, ((zoidfs_handle_t *)i2)->data, 
         sizeof (((zoidfs_handle_t *)i1)->data)) == 0 ? 1 : 0); 
}

static int dcache_key_hash (gencache_key_t key)
{
   const zoidfs_handle_t * h = (const zoidfs_handle_t *) key; 
   const int * ptr = (int *) &h->data; 
   int init = 66; 
   unsigned int i; 
   assert (sizeof (h->data) % sizeof(int) == 0); 
   for (i=0; i<sizeof (h->data)/sizeof(int); ++i)
   {
      init ^= *ptr++; 
   }
   return init; 
}

dcache_handle dcache_create (int cachesize)
{
   gencache_init_t init;
   dcache_handle h = malloc (sizeof(dcache_instance)); 

   init.max_cache_size = cachesize; 
   init.fn_item_free = dcache_item_free; 
   init.fn_key_free = dcache_key_free; 
   init.fn_key_compare = dcache_key_compare;
   init.fn_key_hash = dcache_key_hash; 

   h->gencache = gencache_init (&init); 
   pthread_mutex_init (&h->lock, 0); 
   return h; 
}

int dcache_destroy (dcache_handle h)
{
   int ret; 
   pthread_mutex_lock (&h->lock); 
   gencache_done (h->gencache); 
   ret = pthread_mutex_unlock (&h->lock); 
   pthread_mutex_destroy (&h->lock);
   free (h); 
   return ret; 
}


/* helper function; needs to be called with the lock held */ 
static int dcache_getfd_helper (dcache_handle h, const zoidfs_handle_t * handle,
      Descriptor * dest)
{
   FDValue * t; 
   int ret = gencache_key_lookup (h->gencache, handle, (gencache_value_t*) &t, &dest->lock); 

   if (ret)
   {
      dest->fd = t->fd; 

      /* we don't set handle on lookup */
      dest->handle = 0; 
   }

   return ret; 
}

/**
 * We do a shallow copy from the data in the cache
 */
int dcache_getfd (dcache_handle h, const zoidfs_handle_t * handle, Descriptor * dest)
{
   int ret; 

   pthread_mutex_lock (&h->lock); 
   ret = dcache_getfd_helper (h, handle, dest); 
   pthread_mutex_unlock (&h->lock); 

   return ret;
}

int dcache_releasefd (dcache_handle h, Descriptor * dest)
{
   return gencache_key_unlock (h->gencache, &dest->lock); 
}

int dcache_removefd (dcache_handle h, const zoidfs_handle_t * handle)
{
   int ret; 
   pthread_mutex_lock (&h->lock); 
   ret = gencache_key_remove (h->gencache, handle); 
   pthread_mutex_unlock (&h->lock); 
   return ret; 
}

/**
 * A copy of *dest (including the handle) is stored in the cache. 
 * Note that dest->handle needs to be valid 
 *
 * If failure because of duplicate handle, checkout entry
 * and return 0. 
 */
int dcache_addfd (dcache_handle h, const zoidfs_handle_t * handle, 
      int fd, Descriptor * dest)
{
   int ret; 
   zoidfs_handle_t * newhandle;
   FDValue * newdest;
   

   /* lock to protect somebody else from adding the same file */
   pthread_mutex_lock (&h->lock); 

   ret = dcache_getfd_helper (h, handle, dest); 
   if (ret)
   {
      /* already present: return entry */ 
      pthread_mutex_unlock (&h->lock); 
      return 0; 
   }
   
   /* can add new entry */ 
   
   newhandle = malloc (sizeof (zoidfs_handle_t)); 
   newdest = malloc (sizeof (FDValue)); 

   *newhandle = *handle; 
   newdest->fd = fd; 
   dest->fd = fd; 
   dest->handle = handle; 

   ret = gencache_key_add (h->gencache, newhandle, newdest, &dest->lock); 
   assert (ret); 

   pthread_mutex_unlock (&h->lock); 

   return 1; 
}


/* Try to open a file descriptor for the specified file.
 * First try R/W, if this fails use R/O 
 * Returns < 0 if failure */
/*static int getfd (const char * filename, int * err, int create, int * created)
{
    if we try to open in r/w mode 
   int mode;

   if (create)
   {
       if create is true first try to create 
      int ret = open (filename, O_RDWR|O_CREAT|O_EXCL); 
      if (ret < 0)
      {
         if (errno != EEXIST)
         {
            if (created) *created = 0; 
            if (err) *err = errno; 
            return -1; 
         }
      }
      else
      {
          create succeeded 
         if (created) *created = 1; 
         if (err) *err = 0; 
         return ret; 
      }

      * create didn't succeed since the file existed already 
       * Fall back to opening the file *
      if (created) *created = 0; 
   }

   * try to open in R/W mode, R/O mode and W/O mode *
   for (mode = 0; mode < 3; ++mode)
   {
      int flags;
      int ret; 
      switch (mode)
      {
         case 0: 
            flags |= O_RDWR;
            break;
         case 1:
            flags |= O_RDONLY;
            break;
         case 2:
            flags |= O_WRONLY; 
            break;
         default:
            assert (0 && "Should not happen!"); 
            exit (1); 
      }; 

      ret = open (filename, flags); 
      if (ret < 0)
      {
         if (errno != EACCES)
         {
            * some other error, give up *
            if (err) *err = errno;
            return -1; 
         }

         * we don't have permission, try other mode *
         continue;   * not strictly needed *
      }
      else
      {
         * success *
         if (err) *err = 0; 
         return ret; 
      }
   }

   * we could get here if we have no permission at all to access the file *
   if (err) *err = errno;
   return -1; 
} */
