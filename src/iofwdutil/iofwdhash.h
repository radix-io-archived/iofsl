#ifndef IOFWD_IOFWDHASH_H
#define IOFWD_IOFWDHASH_H

#include "iofwdbool.h"

struct iofwdhash; 

typedef struct iofwdhash * iofwdh_t;

typedef iofwdbool_t (*iofwdh_comparefunc_t) (void * user, 
      const void * key1, const void * key2);

typedef int (*iofwdh_hashfunc_t) (void * user, int size, 
      const void * key); 

iofwdh_t iofwdh_init (int size, iofwdh_comparefunc_t compare,
      iofwdh_hashfunc_t hashfunc, void * user);

void iofwdh_destroy (iofwdh_t table);

int iofwdh_itemcount (iofwdh_t table); 

iofwdbool_t iofwdh_lookup (iofwdh_t table, void * key, void ** data);
void iofwdh_add (iofwdh_t table, void * key, void * data);
iofwdbool_t iofwdh_remove (iofwdh_t table, void * key, void **data); 


#endif
