#include "handlecache.h"
#include "trie.h"
#include "gen-locks.h"

/*
 * Implement handle cache, emulating stateless open for a POSIX filesystem 
 *
 * Automatically closes&reopens files if needed.
 *
 * NOTE: Handles are only valid in the process that called 'lookup'
 *
 *
 */


struct hc_node
{
   const char * filename; 
   int flags; 
   hc_value_t  handle; 
   unsigned int lastused; 
};


/*==========================================*/
static unsigned int hc_counter = 0; 
static handlecache_open_t hc_openfunc = 0; 
static handlecache_close_t hc_closefunc = 0; 
static unsigned int hc_maxopen = 100; 
/*==========================================*/

int handlecache_init (int size, handlecache_open_t openfunc, 
      handlecache_close_t closefunc)
{
   hc_openfunc = openfunc; 
   hc_closefunc = closefunc; 

   if (size)
   {
      hc_maxopen = size; 
   }

   return TRUE; 
}


inline static void handlecache_usenode (struct hc_node * node)
{
   node->lastused = ++hc_counter; 
}

int  handlecache_get (const char * path, int flags, hc_item_handle_t * handle,
      hc_value_t * user)
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
