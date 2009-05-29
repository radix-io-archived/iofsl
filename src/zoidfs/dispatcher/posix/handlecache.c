#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <pthread.h>

#include "zoidfs-util.h"
#include "handlecache.h"
#include "trie.h"
#include "hash-table.h"
#include "env-parse.h"

#ifndef NDEBUG
static int do_debug = 0; 
#define hc_debug(format, ...) if(do_debug) fprintf (stderr, "handlecache: debug: " format, ##__VA_ARGS__)
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
static HashTable * hc_hash = 0; 
static hc_entry_t * hc_first = 0;
static hc_entry_t * hc_last = 0;  
static int          hc_count = 0; 
static int          hc_capacity = 0; 
static handlecache_removefunc_t hc_removefunc = 0; 
static unsigned long hc_miss = 0;
static unsigned long hc_hit = 0;  

static pthread_mutex_t hc_mutex = PTHREAD_MUTEX_INITIALIZER; 
/* ================================= */ 



inline static unsigned long hc_gen_hash (HashTableKey val)
{
   unsigned long ret = 0;
   unsigned int i; 
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

/* needs to be called with lock */
static int checkinvariant ()
{
   assert (hc_count == hash_table_num_entries (hc_hash));
   if (hc_count > 0)
   {
      assert (hc_first); 
      assert (hc_last); 
   }

   if (hc_count == 1)
   {
      assert (hc_first == hc_last); 
   }

   if (hc_count > 1)
   {
      assert (hc_first != hc_last); 
   }

   assert (hc_count <= hc_capacity); 

   return 1; 
}

int handlecache_init (int size, handlecache_removefunc_t remove)
{
   static pthread_mutex_t  mutex = PTHREAD_MUTEX_INITIALIZER; 

   pthread_mutex_lock (&mutex);
   do
   {
      if (hc_hash)
         break; 

#ifndef NDEBUG
      if (env_parse_have_debug("handlecache"))
         do_debug=1; 
#endif
      assert (size > 1); 
      hc_removefunc = remove; 
      hc_first = 0; 
      hc_last = 0; 
      hc_count = 0; 
      hc_capacity = size; 
      hc_hash = hash_table_new (hc_gen_hash, hc_comp_hash); 
   } while (0);

   checkinvariant (); 

   pthread_mutex_unlock (&mutex); 

   return 1; 
}

int handlecache_destroy ()
{
   pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
   HashTableIterator iter; 
   HashTableValue val; 

   pthread_mutex_lock (&mutex); 
   pthread_mutex_lock (&hc_mutex); 

   do
   {
      checkinvariant (); 

      if (!hc_hash)
         break; 

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
      hc_hash = 0; 

   } while (0); 

   pthread_mutex_unlock (&hc_mutex); 
   pthread_mutex_unlock (&mutex); 

   return 1; 
}


/** Needs to be called with lock held */ 
static int handlecache_remove_entry (hc_entry_t * old)
{
   int ret; 

   assert (hash_table_num_entries (hc_hash) == hc_count); 

   assert (checkinvariant()); 

   if (!hc_count)
      return 0; 

   assert (old); 
   assert (hc_last); 
   assert (hc_first); 

   ret = hash_table_remove (hc_hash, old->key); 
   if (!ret)
      return ret; 


   hc_removefunc (&old->file); 

   if (hc_last == hc_first)
   {
      /* removing only remaining entry */
      assert (hc_count == 1); 
      hc_last = hc_first = 0; 
   } 
   else if (old == hc_last)
   {
      /* removing last entry; at least one other entry */ 
      assert (hc_last->prev); 
      assert (hc_last->next == 0); 
      hc_last = hc_last->prev; 
      hc_last->next = 0; 
   }
   else if (old == hc_first)
   {
      /* removing first entry; at least one other entry */ 
      assert (hc_first->prev == 0); 
      assert (hc_first->next); 
      hc_first = hc_first->next; 
      hc_first->prev = 0; 
   }
   else
   {
      /* old != first or last, must have predecessor and successor */
      assert (hc_count > 2); 
      assert (old->prev); 
      assert (old->next); 
      old->prev->next = old->next; 
      old->next->prev = old->prev; 
   }

   /* free storage */ 
   free (old->key); 
   free (old); 

   --hc_count; 

   return ret; 
}

int handlecache_purge (const zoidfs_handle_t * k)
{
   int ret = 0; 

   pthread_mutex_lock (&hc_mutex); 

   do
   {
      assert (checkinvariant ()); 

      hc_entry_t * e = (hc_entry_t *) hash_table_lookup (hc_hash, (void*) k);
      if (!e)
         break; 
      ret=handlecache_remove_entry (e); 
   } while (0); 

   pthread_mutex_unlock (&hc_mutex); 

   return ret; 
}

int handlecache_add (const zoidfs_handle_t * k, hc_item_value_t * val)
{
   int ret = 1; 

   pthread_mutex_lock (&hc_mutex);

   do
   {
      assert (checkinvariant ()); 

      /* first try lookup */
      assert (!hash_table_lookup (hc_hash, (void*)k));

      
      /* See if we need to remove one */
      if (hc_count == hc_capacity)
      {
         handlecache_remove_entry (hc_last); 
      }

      hc_entry_t * n = malloc (sizeof (hc_entry_t)); 
      zoidfs_handle_t * key = (zoidfs_handle_t*) malloc (sizeof(zoidfs_handle_t)); 

      /* copy the key for later */ 
      memcpy (key, k, sizeof (zoidfs_handle_t)); 

      n->file = *val; 
      n->key = key; 


      hc_debug ("Adding %s [current size=%u/capacity=%u]\n", zoidfs_handle_string(k),hc_count,
            (unsigned) hc_capacity); 

      hash_table_insert (hc_hash, key, n); 
      
      ++hc_count; 

      /* add to front of the list */ 
      n->prev = 0; 
      n->next = hc_first; 
      if (hc_first)
         hc_first->prev = n; 
      hc_first = n; 

      if (!hc_last)
      {
         /* if there was no hc_last or hc_first this is the first one we add */ 
         assert (hc_count == 1); 
         hc_last = n;
         assert(hc_last->next == 0); 
         assert(hc_last->prev == 0); 
         assert(hc_last == hc_first); 
      }

   } while (0); 

   pthread_mutex_unlock (&hc_mutex); 

   return ret; 
}

int handlecache_lookup (const zoidfs_handle_t * key, hc_item_value_t * dst)
{
   int ret = 1;
   HashTableValue  val;
   
   pthread_mutex_lock (&hc_mutex); 

   do
   {
      assert (checkinvariant ()); 

      val = hash_table_lookup (hc_hash, (HashTableKey) key);
      if (!val)
      {
         ++hc_miss; 
         hc_debug ("miss for %s  [misses %lu/total %lu | size=%u capacity=%u]\n", 
               zoidfs_handle_string(key), hc_hit, hc_hit+hc_miss,
               (unsigned)hc_count, (unsigned)hc_capacity); 
         ret = 0; 
         break; 
      }

      ++hc_hit; 

      hc_debug ("hit for %s  [hit %lu/ total %lu, size=%lu, capacity=%lu]\n", 
            zoidfs_handle_string(key), hc_hit, hc_hit+hc_miss,
            (long unsigned int) hc_count, (long unsigned int) hc_capacity); 

      hc_entry_t * e = (hc_entry_t *) val; 
      *dst = e->file; 
         
      /* there hase to be a hc_first otherwise we wouldn't be able to do a
       * lookup on it */
      assert (hc_first); 

      /* only move when the element is not 
       * already at the head of the list
       * (note that when there is only one element it is automatically the head
       * of the list)
       */
      if (hc_first != e)
      {
         /* e is not hc_first and must have  a predecessor */ 
         assert (e->prev); 

         /* disconnect from current position */ 
         e->prev->next = e->next; 

         if (e->next)
         {
            e->next->prev = e->prev; 
         }
         else
         {
            /* if e has no next, it has to be hc_last */
            /* if the element is the last element update hc_last */
            assert (e == hc_last); 

            /* e must have a prev otherwise it would be hc_first 
             * and we wouldn't have gotten here */ 
            assert (e->prev); 

            hc_last = e->prev; 
         }

         e->next = e->prev = 0; 

         /* reattach at the front of the list */ 
         e->next = hc_first; 

         assert (hc_first->prev == 0); 
         hc_first->prev = e; 
         hc_first = e; 
      }
   } while (0); 
      
   assert (checkinvariant ()); 

   pthread_mutex_unlock (&hc_mutex); 

   return ret; 
}
