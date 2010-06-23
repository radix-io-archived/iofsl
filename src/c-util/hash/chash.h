#ifndef C_UTIL_HASH_CHASH_H
#define C_UTIL_HASH_CHASH_H

#include <stddef.h>

struct HashFunction;

typedef struct HashFunction * HashHandle;


/**
 * Return number of bytes in return ofhash function
 */
size_t chash_getsize (HashHandle h);

/**
 * Return handle for the named hash function
 * Returns 0 if not found.
 */
HashHandle chash_lookup (const char * name);

/**
 * Reset internal state of given hash function
 */
int chash_reset (HashHandle h);

/**
 * Hash given data
 * Returns number of bytes processed, or <0 on error
 */
int chash_process (HashHandle h, const void * data, size_t bytes);

/**
 * Free the given handle. Releases internal storage.
 */
int chash_free (HashHandle * h);

/**
 * Retrieve hash. After this, reset needs to be called before more data can be
 * hashed. Return number of bytes written
 */
int chash_get (HashHandle h, void * dest, size_t destsize);

/**
 * Must be called before any other chash function.
 */
int chash_init ();

int chash_done ();


/**
 * For implementing concrete hash functions -- not part of the public
 * interface
 */

struct HashFunctionImpl
{
   int    (*reset)   (HashHandle h);
   int    (*getsize) (HashHandle h);
   int    (*init)    (HashHandle h);
   int    (*free)    (HashHandle h);
   int    (*process) (HashHandle h, const void * data, size_t bytes);
   int    (*get)     (HashHandle h, void * dest, size_t destsize);
};

struct HashFunction
{
   struct HashFunctionImpl * impl;
   void * data;
};

#endif
