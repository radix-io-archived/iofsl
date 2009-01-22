#include <unistd.h>
#include <assert.h>
#include <string.h>
#include "handlecache.h"
#include "trie.h"
#include "gen-locks.h"
#include "hash-table.h"
#include "zoidfs-util.h"
#include <stdio.h>

#ifndef NDEBUG
#define hc_debug(format, ...) fprintf (stderr, "handlecache: debug: " format, ##__VA_ARGS__)
#else
#define hc_debug(format, ...) {}
#endif


struct hc_entry; 

typedef struct hc_entry_t hc_entry_t; 

struct hc_entry_t
{
   hc_entry_t * next; 
   hc_entry_t * prev; 
   hc_item_value_t file; 
   zoidfs_handle_t * key; 
};


/* ================================== */
static HashTable * hc_hash; 
static hc_entry_t * hc_first;
static hc_entry_t * hc_last; 
static int          hc_count; 
static int          hc_capacity; 
static handlecache_removefunc_t hc_removefunc; 
static unsigned long hc_miss;
static unsigned long hc_hit; 
/* ================================= */ 



inline static unsigned long hc_gen_hash (HashTableKey val)
{
   unsigned long ret = 0;
   int i; 
   const unsigned int loop = sizeof(zoidfs_handle_t)/sizeof(unsigned long); 
   const unsigned long * ptr = val; 
   assert (loop); 
   for (i=0; i<loop; ++i)
      ret ^= *ptr++; 
   return ret; 
}

inline static int hc_comp_hash (HashTableKey v1, HashTableKey v2)
{
   return memcmp (v1, v2, sizeof(zoidfs_handle_t)) == 0; 
}

inline static void hc_free_hash_key (HashTableKey k)
{
   free ((zoidfs_handle_t*) (k)); 
}

inline static void hc_free_hash_value (HashTableValue k)
{
   free ((hc_entry_t*) (k)); 
}


int handlecache_init (int size, handlecache_removefunc_t remove)
{
   assert (size > 1); 
   hc_removefunc = remove; 
   hc_first = 0; 
   hc_last = 0; 
   hc_count = 0; 
   hc_capacity = size; 
   hc_hash = hash_table_new (hc_gen_hash, hc_comp_hash); 
   //hash_table_register_free_functions (hc_hash, 
   //      hc_free_hash_key, hc_free_hash_value); 
   return 1; 
}

int handlecache_destroy ()
{
   HashTableIterator iter; 
   HashTableValue val; 

   /* walk the whole list and free everything */ 
   assert (hash_table_num_entries (hc_hash) == hc_count); 


   hash_table_iterate (hc_hash, &iter); 
   while ((val = hash_table_iter_next (&iter)))
   {
      hc_entry_t * e = (hc_entry_t *) val; 

      hc_removefunc (&e->file);

      free (e->key); 
      free (e); 
   }

   hash_table_free (hc_hash); 
   hc_first = 0; 
   hc_last = 0; 
   hc_count = 0; 
   return 1; 
}

int handlecache_add (const zoidfs_handle_t * k, hc_item_value_t * val)
{
   /* first try lookup */
   assert (!hash_table_lookup (hc_hash, (void*)k));

   
   hc_entry_t * n = malloc (sizeof (hc_entry_t)); 
   zoidfs_handle_t * key = (zoidfs_handle_t*) malloc (sizeof(zoidfs_handle_t)); 

   /* copy the key for later */ 
   memcpy (key, k, sizeof (zoidfs_handle_t)); 

   n->file = *val; 
   n->key = key; 

   ++hc_count; 
   
   hc_debug ("Adding %s [size=%u]\n", zoidfs_handle_string(k),hc_count); 

   /* add to front of the list */ 
   n->prev = 0; 
   n->next = hc_first; 
   if (hc_first)
      hc_first->prev = n; 
   hc_first = n; 
   
   if (!hc_last)
   {
      assert (hc_count == 1); 
      hc_last = n;
   }

   /* See if we need to remove one */
   if (hc_count > hc_capacity)
   {
      hc_entry_t * old = hc_last; 
      int ret; 

      assert (hc_last); 
      hc_removefunc (&hc_last->file); 
      hc_last = hc_last->prev; 
      hc_last->next = 0; 

      ret = hash_table_remove (hc_hash, old->key); 
      assert (ret); 

      /* free storage */ 
      free (old->key); 
      free (old); 

      --hc_count; 
   }

   return 1; 
}

int handlecache_lookup (const zoidfs_handle_t * key, hc_item_value_t * dst)
{
   HashTableValue  val = hash_table_lookup (hc_hash, (HashTableKey) key);
   if (!val)
   {
      ++hc_miss; 
      hc_debug ("miss for %s  [%lu/%lu]\n", 
            zoidfs_handle_string(key), hc_hit, hc_hit+hc_miss); 
      return 0; 
   }

   ++hc_hit; 
      
   hc_debug ("hit for %s  [%lu/%lu]\n", 
            zoidfs_handle_string(key), hc_hit, hc_hit+hc_miss); 
   
   hc_entry_t * e = (hc_entry_t *) val; 
   *dst = e->file; 
   
   /* disconnect from current position */ 
   if (e->prev)
      e->prev->next = e->next; 
   if (e->next)
      e->next->prev = e->prev; 

   e->next = e->prev = 0; 

   /* reattach at the front of the list */ 
   e->next = hc_first; 
   if (hc_first)
   {
      assert (hc_first->prev == 0); 
      hc_first->prev = e; 
   }
   else
      hc_first = e; 

   return 1; 
}
