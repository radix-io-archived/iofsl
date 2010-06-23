#include <string.h>
#include <malloc.h>

#include "chash.h"

#include "iofwd_config.h"

#include "chash_sha1.h"
#include "chash_none.h"

#ifdef HAVE_ZLIB
#include "chash_adler32.h"
#include "chash_crc32.h"
#endif



struct RegisteredHashFunction
{
   const char * name;
   struct HashFunctionImpl * impl;
};

static struct RegisteredHashFunction hashfunctions[] = 
{
#ifdef HAVE_ZLIB
   { "adler32", &chash_adler32 },
   { "crc32", &chash_crc32 },
#endif
   { "sha1", &chash_sha1 },
   { "none", &chash_none }
};

size_t chash_getsize (HashHandle h)
{
   return h->impl->getsize (h);
}

HashHandle chash_lookup (const char * name)
{
   size_t i;
   for (i=0; i<sizeof(hashfunctions)/sizeof(hashfunctions[0]); ++i)
   {
      if (!strcasecmp (name, hashfunctions[i].name))
      {
         HashHandle n = malloc (sizeof (struct HashFunction));
         n->impl = hashfunctions[i].impl;
         n->data = 0;
         n->impl->init (n);
         return n;
      }
   }
   return 0;
}

int chash_reset (HashHandle h)
{
   return h->impl->reset (h);
}

/**
 * Hash given data
 * Returns number of bytes processed, or <0 on error
 */
int chash_process (HashHandle h, const void * data, size_t bytes)
{
   return h->impl->process (h, data, bytes);
}

/**
 * Free the given handle. Releases internal storage.
 */
int chash_free (HashHandle * h)
{
   int ret = (*h)->impl->free (*h);
   free (h);
   *h = 0;
   return ret;
}

/**
 * Retrieve hash. After this, reset needs to be called before more data can be
 * hashed. Return number of bytes written
 */
int chash_get (HashHandle h, void * dest, size_t destsize)
{
   return h->impl->get (h, dest, destsize);
}


int chash_init ()
{
   return 1;
}

int chash_done ()
{
   return 1;
}
