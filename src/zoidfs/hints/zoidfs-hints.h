#ifndef SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C
#define SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C

#include "zoidfs/zoidfs.h"

#ifdef __cplusplus
/*
 * We put all the zoidfs stuff in the zoidfs namespace
 */
namespace zoidfs
{
   extern "C"
   {
#endif /* __cplusplus */

/* predefined hints */
#define ZOIDFS_HINT_ENABLED (char*)"1"
#define ZOIDFS_HINT_DISABLED (char*)"0"
#define ZOIDFS_ENABLE_PIPELINE (char*)"ZOIDFS_ENABLE_PIPELINE"
#define ZOIDFS_PIPELINE_SIZE (char*)"ZOIDFS_PIPELINE_SIZE"


/* new hint API */

/* zoidfs hints are modeled on the MPI info interface */

/* TODO these are currently just stubs */

int zoidfs_op_hint_create(zoidfs_op_hint_t * hint);
int zoidfs_op_hint_set(zoidfs_op_hint_t hint, char * key, char * value);
int zoidfs_op_hint_delete(zoidfs_op_hint_t hint, char * key);
int zoidfs_op_hint_get(zoidfs_op_hint_t hint, char * key, int valuelen, char * value, int * flag);
int zoidfs_op_hint_get_valuelen(zoidfs_op_hint_t hint, char * key, int * valuelen, int * flag);
int zoidfs_op_hint_get_nkeys(zoidfs_op_hint_t hint, int * nkeys);
int zoidfs_op_hint_get_nthkey(zoidfs_op_hint_t hint, int n, char * key);
int zoidfs_op_hint_dup(zoidfs_op_hint_t oldhint, zoidfs_op_hint_t * newhint);
int zoidfs_op_hint_free(zoidfs_op_hint_t * hint);

/* old hint API */

/* create and initialize the hint list */
zoidfs_op_hint_t * zoidfs_hint_init(int size);

/* add a new (key, value) pair to the hint list */
int zoidfs_hint_add(zoidfs_op_hint_t ** op_hint,
                    char * key,
                    char * value,
                    int value_len,
                    int flags);

/* remove a (key, value) pair from the hint list */
int zoidfs_hint_remove(zoidfs_op_hint_t ** op_hint,
                        char * key,
                        int flags);

/* get a (key, value) pair from the hint list */
char * zoidfs_hint_get(zoidfs_op_hint_t ** op_hint,
                        char * key);

/* destroy the hint list */
int zoidfs_hint_destroy(zoidfs_op_hint_t ** op_hint);

/* remove the hint from the front of the list */
zoidfs_op_hint_t * zoidfs_hint_pop(zoidfs_op_hint_t ** op_hint);

/* print the contents of the hint list */
int zoidfs_hint_print(zoidfs_op_hint_t ** op_hint);

/* get the size of the hint list */
int zoidfs_hint_num_elements(zoidfs_op_hint_t ** op_hint);

/* get the hint at index i in the list */
zoidfs_op_hint_t * zoidfs_hint_index(zoidfs_op_hint_t ** op_hint,
                                        int index);

/* allocate the key buffer */
char * zoidfs_hint_make_key(int key_len);

/* deallocate the key buffer */
int zoidfs_hint_rm_key(char * key);

/* allocate the value buffer */
char * zoidfs_hint_make_value(int value_len);

/* deallocate the value buffer */
int zoidfs_hint_rm_value(char * value);

/* encode the int value into the hint */
void encode_int(char ** pptr,
                void * value);

/* encode the double value into the hint */
void encode_double(char ** pptr,
                void * value);

/* decode the int value stored in the hint */
void decode_int(char ** pptr,
                void * value);

/* decode the double value stored in the hint */
void decode_double(char ** pptr,
                void * value);

#ifdef __cplusplus
    } /* extern */
} /* namespace */
#endif /* __cplusplus */

#endif /* __SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C__ */
