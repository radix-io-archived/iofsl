#include "handlecache.h"
#include "iofwd-hash.h"
#include "gen-locks.h"

struct hc_node; 

typedef struct
{
   const char * filename; 
   int write; 
   int read; 
   hc_node * next; 
   hc_value_t  handle; 
} hc_node; 


/*==========================================*/
static hc_node * hc_cache = 0;  
static handlecache_open_t hc_openfunc = 0; 
static handlecache_close_t hc_closefunc = 0; 
static qhash_table * hc_hash = 0; 
/*==========================================*/

static int hc_hash_comparefunc (void * key, struct qhash_head * link)
{
}

static int hc_hash_hashfunc (void * key, int table_size)
{
   return (*(const int*) (key)) % table_size; 
}


static hc_node * handlecache_newnode ()
{
   return calloc (1, sizeof (hc_node)); 
}

static void handlecache_freenode (const hc_node * node)
{
   if (!node)
      return;
   hc_closefunc (&node->handle); 
   free (node->name); 
   free (node); 
}

int handlecache_init (int size, handlecache_open_t openfunc, 
      handlecache_close_t closefunc)
{
   /* alloc hash table */
   hc_hash = qhash_table (hc_hash_comparefunc,
         hc_hash_hashfunc, 131); 

   /* alloc first node */ 
   hc_node = handlecache_newnode (); 
}

int handlecache_destroy ()
{
   // close all remaining handles
   hc_node * cur = hc_cache;
   while (cur)
   {
      hc_node * next = cur; 
      handlecache_freenode(cur); 
      cur = next; 
   }
   hc_cache = 0; 
}

hc_value_t  handlecache_get (const char * path, int flags)
{
   // walk list
   const hc_node * cur = hc_cache;
   const hc_node * last = 0; 
   int count = 0; 
   while (cur)
   {
      if (cur->name && !strcmp(cur->name, path))
      {
         /* found */


         if ((cur->flags & flags) == flags)
                return cur->handle; 
         /* flags do not match */
         hc_openfunc (path, &cur->handle, flags, true); 
      }
      cur = cur->next; 
      ++count; 
   }
   /* not found */
   if (count == hc_cache_size)
   {
      /* need to make room first */
      
   }

   cur = handlecache_newnode ();
   cur->name = strdup (path); 
   cur-> 
}


