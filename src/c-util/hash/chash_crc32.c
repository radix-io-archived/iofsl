#include <zlib.h>
#include <malloc.h>
#include <stdint.h>

#include "src/c-util/tools.h"

#include "chash_crc32.h"

static int chash_crc32_reset (HashHandle h)
{
   *(uint32_t*) h->data = crc32(0, 0, 0);
   return 1;
}

static int chash_crc32_getsize (HashHandle UNUSED(h))
{
   return 4;
}

static int chash_crc32_init (HashHandle h)
{
   h->data = malloc (sizeof (uint32_t));
   chash_crc32_reset (h);
   return 1;
}

static int chash_crc32_free (HashHandle h)
{
   free (h->data);
   return 1;
}

static int chash_crc32_process (HashHandle h, const void * data, size_t bytes)
{
   *(uint32_t *) h->data = crc32 (*(uint32_t *) h->data, data, bytes);
   return bytes;
}

static int chash_crc32_get (HashHandle h, void * dest, size_t destsize)
{
   ALWAYS_ASSERT(destsize >= sizeof(uint32_t));
   *(uint32_t *) dest = *(uint32_t *) h->data;
   return sizeof(uint32_t);
}

struct HashFunctionImpl chash_crc32 = {
   .reset = &chash_crc32_reset,
   .getsize= &chash_crc32_getsize,
   .init = &chash_crc32_init,
   .free = &chash_crc32_free,
   .process = &chash_crc32_process,
   .get = &chash_crc32_get
};
