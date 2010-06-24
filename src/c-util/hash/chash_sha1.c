#include <malloc.h>
#include <string.h>

#include "c-util/tools.h"
#include "c-util/sha1.h"
#include "chash_sha1.h"

static int chash_sha1_reset (HashHandle h)
{
   SHA1Reset ((SHA1Context*) h->data);
   return 1;
}

static int chash_sha1_getsize (HashHandle UNUSED(h))
{
   return 20;
}

static int chash_sha1_init (HashHandle h)
{
   h->data = malloc (sizeof (SHA1Context));
   chash_sha1_reset (h);
   return 1;
}

static int chash_sha1_free (HashHandle h)
{
   free ((SHA1Context*) h->data);
   h->data = 0;
   return 1;
}

static int chash_sha1_process (HashHandle h, const void *
      data, size_t bytes)
{
   SHA1Input ((SHA1Context *) h->data, (unsigned char*) data, bytes);
   return bytes;
}

static int chash_sha1_get (HashHandle h, void * dest,
      size_t destsize)
{
   ALWAYS_ASSERT(destsize >= 20);
   memcpy (dest, &((SHA1Context*) h->data)->Message_Digest[0], 20);
   return 0;
}

struct HashFunctionImpl chash_sha1 = {
   .reset = &chash_sha1_reset,
   .getsize= &chash_sha1_getsize,
   .init = &chash_sha1_init,
   .free = &chash_sha1_free,
   .process = &chash_sha1_process,
   .get = &chash_sha1_get
};
