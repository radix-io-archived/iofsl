#ifndef __SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C__
#define __SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C__

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
                        char * key);

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