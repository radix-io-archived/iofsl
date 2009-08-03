#ifndef ZOIDFS_POSIX_HANDLECACHE_H
#define ZOIDFS_POSIX_HANDLECACHE_H

#include <stdint.h>
#include "zoidfs.h"

/*
 * General cache; Expires least recently used item
 */

typedef void * gencache_handle; 

typedef const void * gencache_key_t; 
typedef void * gencache_value_t; 

typedef void * gencache_lock_info; 


typedef int (*gencache_item_free_function_t) (void * item);
typedef int (*gencache_key_free_function_t) (void * item);
typedef int (*gencache_compare_function_t) (const void * item1, const void * item2); 
typedef int (*gencache_hash_function_t) (gencache_key_t key); 

typedef struct 
{
   unsigned int max_cache_size; 

   gencache_item_free_function_t      fn_item_free; 
   gencache_key_free_function_t      fn_key_free; 
   gencache_compare_function_t   fn_key_compare; 
   gencache_hash_function_t      fn_key_hash; 
   
} gencache_init_t; 

gencache_handle gencache_init (const gencache_init_t * init); 
int gencache_done (gencache_handle handle); 

/*
 * Locking prevents a key from being expired 
 */
int gencache_key_unlock (gencache_handle handle, gencache_lock_info * info); 


/* Add key to cache. If info isn't NULL, lock the entry */
int gencache_key_add (gencache_handle handle, gencache_key_t key,
      gencache_value_t value, gencache_lock_info * info); 

/*
 * If the key is removed, block until the key becomes free
 */
int gencache_key_remove (gencache_handle handle, gencache_key_t key);

/*
 * Return >0 if key was found; If lock is non-zero, the item is locked
 * if found and needs to be unlocked using unlock
 */
int gencache_key_lookup (gencache_handle handle, gencache_key_t key,
      gencache_value_t * value, gencache_lock_info * lock); 

/*
 * Return >0 if key was found. Like lookup but protects from expiring
 */
int gencache_key_lookup_refresh (gencache_handle handle, gencache_key_t key,
      gencache_value_t * value, gencache_lock_info * lock);

#endif
