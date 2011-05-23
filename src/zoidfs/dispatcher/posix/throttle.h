#ifndef ZOIDFS_POSIX_THROTTLE_H
#define ZOIDFS_POSIX_THROTTLE_H

#include "zoidfs/zoidfs.h"

struct throttle_entry_t;
struct throttle_t;

typedef struct throttle_t * throttle_handle_t;

typedef struct throttle_entry_t * throttle_entry_handle_t;


typedef void (*throttle_user_free_t) (void * user);


/* Initialize new throttle structure in h */
void throttle_init (throttle_handle_t * h);

/* Free throttle structure in h */
void throttle_done (throttle_handle_t * h);

/* Set the free function for throttle entries */
void throttle_set_user_free (throttle_handle_t h, throttle_user_free_t f);

/* Set user data */
void throttle_set_user (throttle_handle_t h, throttle_entry_handle_t e,
      void * data);

/* Return user data */
void * throttle_get_user (throttle_handle_t h, throttle_entry_handle_t e);

/* Try to obtain entry and return it.
 * Sets obtained to 1 if this is the first call to throttle_try.
 * All following calls to throttle_try will block until the throttle_release
 * is called, at which point they will all return with obtained = 0.
 *
 * In both cases, the entry is returned and throttle_release needs to be
 * called.
 */
throttle_entry_handle_t throttle_try (throttle_handle_t h,
      const zoidfs_handle_t * handle, int * obtained);

/**
 * Release entry and wake any blocked callers on throttle_try
 */
void throttle_release (throttle_handle_t h, const zoidfs_handle_t * handle,
      throttle_entry_handle_t e);


#endif
