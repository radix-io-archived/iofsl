#include "handlecache.h"
#include "iofwdhash.h"
#include "gen-locks.h"

/*
 * Implement handle cache, emulating stateless open for a POSIX filesystem 
 *
 * Automatically closes&reopens files if neded.
 *
 * Is optimized for cases where the open handle limit will not be reached.
 *
 */


struct hc_node
{
   const char * filename; 
   int write; 
   int read; 
   hc_value_t  handle; 
   unsigned int lastused; 
};


/*==========================================*/
static unsigned int hc_counter = 0; 
static handlecache_open_t hc_openfunc = 0; 
static handlecache_close_t hc_closefunc = 0; 
static iofwdh_t hc_hash; 
/*==========================================*/

static int hc_hash_comparefunc (void * user, const void * key1, 
      const void * key2)
{
   return key1 == key2; 
}

static int hc_hash_hashfunc (void * user, int size, const void * key)
{
   /* long should be same size as pointer on all architectures */
   return (long) key % (int) size; 
}

int handlecache_init (int size, handlecache_open_t openfunc, 
      handlecache_close_t closefunc)
{
   /* alloc hash table */
   hc_hash = iofwdh_init (131, hc_hash_comparefunc,
         hc_hash_hashfunc, 0); 
}

int handlecache_destroy ()
{
}

hc_value_t  handlecache_get (const char * path, int flags)
{
}


static iofwdbool_t handlecache_makeroom_examine (void * user, 
      const void * key, void * data)
{

   return TRUE; 
}

/*
 * Walk the whole hash table and find the least recently used
 * file handle (and close it)
 */
static void handlecache_makeroom ()
{


}
