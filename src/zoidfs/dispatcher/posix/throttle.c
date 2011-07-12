#include "throttle.h"

#include "c-util/hash-table.h"
#include "c-util/tools.h"

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <malloc.h>

struct throttle_entry_t
{
   zoidfs_handle_t       handle;
   pthread_cond_t        cond;
   unsigned int          waiters;
   void *                user;
   int                   active;
};

struct throttle_t
{
   pthread_mutex_t      lock;
   HashTable       *    list;
   throttle_user_free_t freefunc;
};


static void throttle_free_key (void * UNUSED(user), HashTableKey val)
{
   free (val);
}

static unsigned long throttle_hash (void * UNUSED(user), HashTableKey value)
{
   unsigned long v = 0;
   char * val = (char*) &v;
   unsigned int i;
   const char * h = (const char *) value;
   unsigned int pos = 0;

   for (i=0; i<sizeof (zoidfs_handle_t); ++i)
   {
      val[pos++] ^= *h++;
      if (pos >= sizeof (v))
         pos = 0;
   }
   return v;
}

static int throttle_compare (void * UNUSED(user), HashTableKey v1, HashTableKey v2)
{
   return (memcmp (v1, v2, sizeof (zoidfs_handle_t)) == 0);
}

void throttle_init (throttle_handle_t * newh)
{
   throttle_handle_t h = malloc (sizeof (struct throttle_t));
   pthread_mutex_init (&h->lock, 0);
   h->list = hash_table_new (throttle_hash, &throttle_compare);
   hash_table_register_free_functions (h->list, &throttle_free_key, 0);
   h->freefunc = 0;
   *newh = h;
}

void throttle_done (throttle_handle_t * oldh)
{
   throttle_handle_t h = *oldh;
   *oldh= 0;
   pthread_mutex_destroy (&h->lock);
   hash_table_free (h->list);

   free(h);
}

/* needs to be called with h->lock held */
static void throttle_remove_user (throttle_handle_t h, struct throttle_entry_t * o)
{
   assert(o->waiters);

   if (--o->waiters)
      return;

   if (h->freefunc)
      h->freefunc (o->user);

   pthread_cond_destroy (&o->cond);
   free (o);
}

static void throttle_add_user (struct throttle_entry_t * o)
{
   ++o->waiters;
}

throttle_entry_handle_t  throttle_try (throttle_handle_t h,
      const zoidfs_handle_t * handle, int * obtained)
{
   HashTableValue val;

   pthread_mutex_lock (&h->lock);
   val = hash_table_lookup (h->list, (HashTableKey) handle);
   if (!val)
   {
      zoidfs_handle_t * newh = malloc (sizeof (zoidfs_handle_t));
      *newh = *handle;
      struct throttle_entry_t * e = malloc (sizeof (struct throttle_entry_t));
      e->handle = *handle;
      e->waiters = 0;
      e->user = 0;
      e->active = 1;
      pthread_cond_init (&e->cond, 0);
      throttle_add_user (e);
      hash_table_insert (h->list, newh, e);
      *obtained = 1;
      pthread_mutex_unlock (&h->lock);
      return e;
   }
   else
   {
      struct throttle_entry_t * e = val;
      /* somebody is already opening this file 
         wait on the condition variable
       */
      throttle_add_user (e);
      /*zoidfs_debug ("Waiting for release of handle (%u)...\n", e->waiters);*/
      *obtained = 0;

      /* This check is here in case a spurious wake up happens.
       * Until the user calls throttle_release on this entry, e->active is set
       * to 1, so if we wakeup and e->active is still 1, we should wait a bit
       * longer...
       */
      while (e->active)
      {
         pthread_cond_wait (&e->cond, &h->lock);
      }
      pthread_mutex_unlock (&h->lock);

      /* Note: we return the entry we found during our first lookup; This
       * entry is no longer in the hash table and only the callers that were
       * already blocked see the old entry. Any new caller will be able to
       * insert the entry and create a new one for any new blocker that might
       * come along.
       */
      return e;
   }
}

void throttle_release (throttle_handle_t h, const zoidfs_handle_t * handle,
      throttle_entry_handle_t entry)
{
   struct throttle_entry_t * o = (struct throttle_entry_t *) entry;
   struct throttle_entry_t * e;

   pthread_mutex_lock (&h->lock);

   if (o->active)
   {
      /* This is the owner releasing the entry */
      e = hash_table_lookup (h->list, (HashTableKey) handle);
      assert (e == o);
      hash_table_remove (h->list, (HashTableKey) handle);
      pthread_cond_broadcast (&o->cond);
      o->active = 0;
   }

   /* we know that nobody will be waiting on the condition variable here since
    * all waiters were released, and nobody can try to wait on it again until
    * we release the lock, so it is safe to destroy the condition variable
    * here (might happen in remove_user if this is the last user)
    */
   throttle_remove_user (h, o);

   pthread_mutex_unlock (&h->lock);
}

void throttle_set_user (throttle_handle_t h, throttle_entry_handle_t e, void * data)
{
   pthread_mutex_lock (&h->lock);
   e->user = data;
   pthread_mutex_unlock (&h->lock);
}

void throttle_set_user_free (throttle_handle_t h, throttle_user_free_t f)
{
   pthread_mutex_lock (&h->lock);
   h->freefunc = f;
   pthread_mutex_unlock (&h->lock);
}

void * throttle_get_user (throttle_handle_t h, throttle_entry_handle_t e)
{
   void * data;
   pthread_mutex_lock (&h->lock);
   data = e->user;
   pthread_mutex_unlock (&h->lock);
   return data;
}


