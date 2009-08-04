#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>

#include "gencache.h"
#include "hash-table.h"
#include "env-parse.h"

#ifndef NDEBUG
static int do_debug = 0; 
#define hc_debug(format, ...) if(do_debug) fprintf (stderr, "gencache: debug: " format, ##__VA_ARGS__)
#else
#define hc_debug(format, ...) {}
#endif


struct gencache_priv_value_t; 

typedef struct gencache_priv_value_t gencache_priv_value_t; 

struct gencache_priv_value_t
{
   gencache_priv_value_t * next; 
   gencache_priv_value_t * prev; 
   gencache_key_t          key; 
   gencache_value_t        value; 
   pthread_mutex_t         lock; 
   sig_atomic_t            refcount; 
};

typedef struct 
{
   pthread_mutex_t         lock; 
   pthread_cond_t          cond;  /* used to signal the cleanup  */
   unsigned int            capacity; 
   unsigned int            count;

   gencache_key_free_function_t      fn_key_free; 
   gencache_item_free_function_t      fn_item_free; 
   gencache_compare_function_t   fn_key_compare; 
   gencache_hash_function_t      fn_key_hash; 

   HashTable   *           hash; 

   gencache_priv_value_t  * recycle_head; 
   gencache_priv_value_t  * recycle_tail; 

} gencache_instance_t; 


/* ======================================================================== */

static unsigned long gencache_key_hash (void * userptr, HashTableKey val)
{
   gencache_instance_t * gc = (gencache_instance_t *) userptr;
   return gc->fn_key_hash (val); 
}

static int gencache_key_compare (void * userptr, HashTableKey value1, 
      HashTableKey value2)
{
   gencache_instance_t * gc = (gencache_instance_t *) userptr;
   return gc->fn_key_compare (value1, value2);    
}

static void gencache_hash_key_free (void * userptr, void * item)
{
   gencache_instance_t * gc = (gencache_instance_t *) userptr;
   if (gc->fn_key_free) 
      gc->fn_key_free (item); 
}

static void gencache_hash_item_free (void * userptr, void * item)
{
   gencache_instance_t * gc = (gencache_instance_t *) userptr;
   gencache_priv_value_t * i = (gencache_priv_value_t *) item; 
   if (gc->fn_item_free) 
      gc->fn_item_free (i->value); 
   assert (!i->refcount); 
   free (item); 
}

gencache_handle gencache_init (const gencache_init_t * init)
{
   gencache_instance_t * gc = malloc (sizeof(gencache_instance_t));

   pthread_mutex_init (&gc->lock, 0); 
   gc->capacity = init->max_cache_size; 
   gc->count = 0; 
   gc->fn_key_free = init->fn_key_free; 
   gc->fn_item_free = init->fn_item_free; 
   gc->fn_key_compare = init->fn_key_compare;
   gc->fn_key_hash = init->fn_key_hash; 
   gc->recycle_head = 0; 
   gc->recycle_tail = 0; 

   gc->hash = hash_table_new (gencache_key_hash, gencache_key_compare); 
   hash_table_set_user (gc->hash, gc); 

   hash_table_register_free_functions (gc->hash, gencache_hash_key_free, 
         gencache_hash_item_free); 

   return gc; 
}

int gencache_done (gencache_handle handle)
{
   gencache_instance_t * gc = handle; 
   pthread_mutex_lock (&gc->lock); 
   do
   {
      hash_table_free (gc->hash); 
   } while (0); 

   pthread_mutex_unlock (&gc->lock); 
   pthread_mutex_destroy (&gc->lock); 
   free (gc); 

   return 1; 
}

int gencache_key_unlock (gencache_handle handle, gencache_lock_info * info)
{
   gencache_instance_t * gc = (gencache_instance_t *) handle;
   gencache_priv_value_t * item = (gencache_priv_value_t *) info; 

   pthread_mutex_lock (&gc->lock); 

   assert (item); 

   /* Don't need to lock the hash table: this item cannot be removed until
    * unlocked */

   pthread_mutex_lock (&item->lock); 
   assert (item->refcount); 
   --item->refcount; 
   pthread_mutex_unlock (&item->lock); 

   /**
    * Inform threads waiting on the condition variable: this could be:
    *    * a thread waiting to recycle a slot
    *    * a thread waiting to remove a locked item. 
    * If at least one thread of both categories is waiting, we need to wake
    * them both. Hence the broadcast.
    **/
   pthread_cond_broadcast (&gc->cond); 
   pthread_mutex_unlock (&gc->lock); 

   return 1; 
}

/* needs to be called with global lock */
static void gencache_recycle_remove (gencache_instance_t * gc, gencache_priv_value_t * item)
{
   /* list cannot be empty */
   assert (gc->recycle_head);
   assert (gc->recycle_tail); 
   

   if (gc->recycle_head == item && gc->recycle_tail == item)
   {
      /* removing only element in list */
      item->next = item->prev = 0; 
      gc->recycle_head = gc->recycle_tail = 0; 
      return; 
   }
   
   /* item has to be connected right now and is not the only one in the list */
   assert (item->next || item->prev); 

   if (!item->next)
   {
      /* item it the tail */
      assert (gc->recycle_tail == item); 
      /* item is not the only one */
      assert (item->prev); 
      assert (item->prev->next == item); 

      item->prev->next = 0; 
      gc->recycle_tail = item->prev; 

      item->next = item->prev = 0; 

      return; 
   }

   if (!item->prev)
   {
      assert (gc->recycle_head == item); 
      assert (item->next); 
      item->next->prev = 0; 
      gc->recycle_head = item->next; 
      item->next = item->prev = 0; 
      return; 
   }

   /* item is in the middle of the list */
   assert (item->next && item->prev); 
   assert (gc->recycle_head != item); 
   assert (gc->recycle_tail != item); 
   item->next->prev = item->prev;
   item->prev->next = item->next; 
   item->prev = item->next = 0; 
}

/* Needs to be called with global lock; Item should be disconnected */
static void gencache_recycle_add_tail (gencache_instance_t * gc, gencache_priv_value_t * item)
{
   if (!gc->recycle_tail)
   {
      /* list is empty */
      assert (!gc->recycle_head); 
      gc->recycle_tail = item; 
      gc->recycle_head = item; 
      item->next = 0; 
      item->prev = 0; 
      return; 
   }

   assert(!gc->recycle_tail->next); 
   gc->recycle_tail->next = item;
   item->next = 0; 
   item->prev = gc->recycle_tail; 
   gc->recycle_tail = item; 
}

/**
 * Move the item to the tail of the recycle list
 * Needs to be called with global lock held
 */
static void gencache_item_refresh (gencache_instance_t * gc, gencache_priv_value_t * item)
{
   /* if the list is not empty, and the item is already at the tail don't do
    * anything */
   if (gc->recycle_tail == item)
      return;

   /* if the item is connected, first disconnect from the list */
   if (item->next != 0 && item->prev != 0 && gc->recycle_head)
      gencache_recycle_remove (gc, item); 

   /* Add to the tail of the list */
   gencache_recycle_add_tail (gc, item); 
}


/** Remove an entry from the cache.
 * If all entries are locked, sleep until one is unlocked 
 * Needs to be called with lock held, but can release the lock and sleep.
 * Returns with lock held. 
 */
static void gencache_recycle (gencache_instance_t * gc)
{
   gencache_priv_value_t * item;
   
 restart:
   item = gc->recycle_head; 
   if (!gc->count)
   {
      assert (!gc->recycle_head); 
      assert (!gc->recycle_tail); 
      return;
   }

   do
   {
      if (!item->refcount)
      {
         int i; 
         /* we can recycle this item */
         /* disconnect from recycle list */
         gencache_recycle_remove (gc, item); 

         assert (gc->count); 
         --gc->count;
         i = hash_table_remove (gc->hash, item->key); 
         assert (i); 
         return; 
      }
   } while (item); 

   /* could not find an unlocked item */
   /* sleep until somebody unlocked something and retry */
   /* NOTE: there is probably an issue here when a thread is blocking on
    * remove_item and another thread is blocked here */
   pthread_cond_wait (&gc->cond, &gc->lock); 
   goto restart; 
}

int gencache_key_add (gencache_handle handle, gencache_key_t key, gencache_value_t value, 
      gencache_lock_info * lock)
{
   int ret = 1;  
   int tmp; 
   gencache_instance_t * gc = (gencache_instance_t *) handle;
   gencache_priv_value_t * item = malloc (sizeof (gencache_priv_value_t)); 


   item->next = item->prev = 0; 
   item->key = key;
   item->value = value; 
   pthread_mutex_init (&item->lock, 0); 
   item->refcount = 0; 


   pthread_mutex_lock (&gc->lock); 
   do
   {
      /* check if we have space */
      while (gc->count == gc->capacity)
      {
         /* try to remove an item */
         gencache_recycle (gc); 
      }

      assert ((unsigned int) gc->count == 
            (unsigned int) hash_table_num_entries (gc->hash)); 
      assert (gc->count != gc->capacity); 

      tmp = hash_table_insert (gc->hash, key,
            item);
      assert (tmp); 

      ++gc->count; 

      gencache_item_refresh (gc, item); 

   } while (0); 


   if (lock)
   {
      *lock = item; 
      /* inc refcount on item */
      pthread_mutex_lock (&item->lock); 
      ++item->refcount; 
      pthread_mutex_unlock (&item->lock); 
   }


   pthread_mutex_unlock (&gc->lock); 

   return ret;  
}

/*
 * Needs to be called with lock held
 */
static gencache_priv_value_t * gencache_lookup_internal (gencache_instance_t * gc, 
      gencache_key_t key)
{
   return (gencache_priv_value_t *) hash_table_lookup (gc->hash, key); 
}


int gencache_key_remove (gencache_handle handle, gencache_key_t key)
{
   gencache_instance_t * gc = (gencache_instance_t *) handle;
   int ret = 1; 
   int i; 

   pthread_mutex_lock (&gc->lock); 
   do
   {
      /* lookup-and-lock requires the global lock; So if the item is free now,
       * nobody can lock it while we're busy here */
      gencache_priv_value_t * item = gencache_lookup_internal (gc, key); 
      if (!item)
      {
         /* item is not in the cache; we're done */
         ret = 0; 
         break; 
      }

      /* item is in there: cannot be removed as long we have the lock */
      /* check if the item itself is used (refcount != 0) */
      while (item->refcount)
      {
         /** give up the lock and wait until somebody unlocks it. 
          * NOTE: By doing this, somebody else could also try to remove it
          * (and succeed!) Because of this, we have to relookup the item.
          * (Even more annoying would be if while we sleep, somebody removes 
          * it and somebody else adds it again, causing us to remove a
          * different entry than we originally would.
          **/
         pthread_cond_wait (&gc->cond, &gc->lock); 
         gencache_priv_value_t * item = gencache_lookup_internal (gc, key); 
         if (!item)
         {
            /* item is not in the cache; we're done */
            ret = 0; 
            break; 
         }
      }

      /* Being here:
       *    1) we have the lock. Nobody can add or remove 
       *    2) The item we're supposed to remove is still here and unlocked 
       * Remove the item from the hash table (which will call the free
       * functions)
       */
      assert (!item->refcount); 

      i = hash_table_remove (gc->hash, key); 
      assert (i); 
         
      --gc->count; 

      assert ((unsigned int) gc->count == 
            (unsigned int) hash_table_num_entries (gc->hash)); 

   } while (0); 
   pthread_mutex_unlock (&gc->lock); 
   return ret; 
}

static int gencache_key_lookup_helper (gencache_handle handle, gencache_key_t key,
      gencache_value_t * value, gencache_lock_info * lock, int refresh)
{
   gencache_instance_t * gc = (gencache_instance_t *) handle;
   gencache_priv_value_t * item;
   int ret = 1; 

   pthread_mutex_lock (&gc->lock); 
   do
   {
      item = gencache_lookup_internal (gc, key); 
      if (!item)
      {
         ret = 0; 
         if (value) *value = 0; 
         if (lock)  *lock = 0; 
         break; 
      }

      if (value)
         *value = item->value; 

      if (lock)
      {
         *lock = item; 
         /* inc refcount on item */
         pthread_mutex_lock (&item->lock); 
         ++item->refcount; 
         pthread_mutex_unlock (&item->lock); 
      }

      if (refresh)
         gencache_item_refresh (gc,item); 

   } while (0); 
   pthread_mutex_unlock (&gc->lock); 
   return ret; 
}

int gencache_key_lookup_refresh (gencache_handle handle, gencache_key_t key,
      gencache_value_t * value, gencache_lock_info * lock)
{
   return gencache_key_lookup_helper (handle, key, value, lock, 1); 
}

int gencache_key_lookup (gencache_handle handle, gencache_key_t key,
      gencache_value_t * value, gencache_lock_info * lock)
{
   return gencache_key_lookup_helper (handle, key, value, lock, 0); 
}

