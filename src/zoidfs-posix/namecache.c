#include <assert.h>
#include "namecache.h"
#include "trie.h"

/*=====================*/
static int namecache_initialized = 0; 
static Trie * namecache_trie = TRIE_NULL; 
/*=====================*/


int namecache_init ()
{
   assert (!namecache_initialized); 

   namecache_trie = trie_new(); 
   namecache_initialized = 1; 
   return 1; 
}

int namecache_destroy ()
{
   assert (namecache_initialized); 

   namecache_initialized = 0; 
   trie_free (namecache_trie); 
   return 1; 
}

/* exclusive: if true and the mapping already exists,
 * return error */ 
int namecache_add (const char * name, const zoidfs_handle_t * handle, 
      int exclusive)
{
   return 1; 
}

int namecache_lookup (const char * name, zoidfs_handle_t * handle)
{
   return 1; 
}

int namecache_invalidate (const char * name)
{
   return 1; 
}
