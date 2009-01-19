#include <string.h>  
#include <malloc.h>
#include <assert.h>
#include "namecache.h"
#include "trie.h"

typedef struct nc_entry_t nc_entry_t; 

struct nc_entry_t
{
   nc_entry_t *      next; 
   nc_entry_t *      prev; 
   zoidfs_handle_t * handle; 
   char *            path; 
}; 

/*=====================*/
static int namecache_initialized    = 0; 
static int namecache_capacity       = 0; 
static int namecache_count          = 0; 
static Trie * namecache_trie_handle = 0; 
static Trie * namecache_trie_path   = 0; 
static nc_entry_t * namecache_first = 0; 
static nc_entry_t * namecache_last  = 0; 
/*=====================*/


int namecache_init (int capacity)
{
   assert (!namecache_initialized); 
   assert (capacity >= 0); 

   namecache_trie_handle = trie_new(); 
   namecache_trie_path = trie_new(); 
   namecache_initialized = 1; 
   namecache_count = 0; 
   namecache_capacity = capacity; 

   return 1; 
}

int namecache_destroy ()
{
   assert (namecache_initialized); 

   namecache_initialized = 0; 
   trie_free (namecache_trie_handle); 
   trie_free (namecache_trie_path); 
   return 1; 
}

/* exclusive: if true and the mapping already exists,
 * return error */ 
int namecache_add (const char * name, const zoidfs_handle_t * handle)
{
   void * mem;
   int ret; 

   if (!namecache_capacity)
      return 1; 

   mem = malloc (sizeof (zoidfs_handle_t)); 
   memcpy (mem, handle, sizeof (handle)); 
   ret = trie_insert (namecache_trie_path,(char*) name, mem); 
   return ret; 
}

int namecache_lookup_handle (const char * name, zoidfs_handle_t * handle)
{
   zoidfs_handle_t * h;

   if (!namecache_capacity || !namecache_count)
      return 0; 
   
   h = trie_lookup (namecache_trie_path,(char*) name);
   
   if (!h)
      return 0; 

   *handle = *(zoidfs_handle_t*) h; 
   return 1; 
}

int namecache_lookup_path (const zoidfs_handle_t * handle,
      char name, int bufsize)
{
   return 0; 
}

int namecache_invalidate_path (const char * name)
{
   if (!namecache_capacity || !namecache_count)
      return 1; 

   return trie_remove (namecache_trie_path, (char*)name); 
}

int namecache_invalidate_handle (const zoidfs_handle_t * handle)
{
   return 0; 
}
