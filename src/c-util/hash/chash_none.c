#include "c-util/tools.h"
#include "chash_none.h"

static int chash_none_reset (HashHandle UNUSED(h))
{
   return 1;
}

static int chash_none_getsize (HashHandle UNUSED(h))
{
   return 0;
}

static int chash_none_init (HashHandle h)
{
   h->data = 0;
   return 1;
}

static int chash_none_free (HashHandle UNUSED(h))
{
   return 1;
}

static int chash_none_process (HashHandle UNUSED(h), const void *
      UNUSED(data), size_t bytes)
{
   return bytes;
}

static int chash_none_get (HashHandle UNUSED(h), void * UNUSED(dest),
      size_t UNUSED(destsize))
{
   return 0;
}

struct HashFunctionImpl chash_none = {
   .reset = &chash_none_reset,
   .getsize= &chash_none_getsize,
   .init = &chash_none_init,
   .free = &chash_none_free,
   .process = &chash_none_process,
   .get = &chash_none_get
};
