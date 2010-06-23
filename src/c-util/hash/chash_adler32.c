#include <zlib.h>
#include <malloc.h>
#include <stdint.h>

#include "src/c-util/tools.h"
#include "chash_adler32.h"

static int chash_adler32_reset (HashHandle h)
{
   *(uint32_t*) h->data = adler32(0, 0, 0);
   return 1;
}

static int chash_adler32_getsize (HashHandle UNUSED(h))
{
   return 4;
}

static int chash_adler32_init (HashHandle h)
{
   h->data = malloc (sizeof (uint32_t));
   chash_adler32_reset (h);
   return 1;
}

static int chash_adler32_free (HashHandle h)
{
   free (h->data);
   return 1;
}

static int chash_adler32_process (HashHandle h, const void * data, size_t bytes)
{
   *(uint32_t *) h->data = adler32 (*(uint32_t *) h->data, data, bytes);
   return bytes;
}

static int chash_adler32_get (HashHandle h, void * dest, size_t destsize)
{
   ALWAYS_ASSERT(destsize >= sizeof(uint32_t));
   *(uint32_t *) dest = *(uint32_t *) h->data;
   return sizeof(uint32_t);
}

struct HashFunctionImpl chash_none = {
   .reset = &chash_adler32_reset,
   .getsize= &chash_adler32_getsize,
   .init = &chash_adler32_init,
   .free = &chash_adler32_free,
   .process = &chash_adler32_process,
   .get = &chash_adler32_get
};
