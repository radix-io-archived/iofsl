#ifndef SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_H
#define SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_H

#include "zoidfs.h"

#ifdef __cplusplus
/*
 * We put all the zoidfs stuff in the zoidfs namespace
 */
namespace zoidfs
{
    namespace hints
    {
   extern "C"
   {
#endif /* __cplusplus */

/* predefined hints */
#define ZOIDFS_HINT_ENABLED (char*)"1"
#define ZOIDFS_HINT_DISABLED (char*)"0"
#define ZOIDFS_ENABLE_PIPELINE (char*)"ZOIDFS_ENABLE_PIPELINE"
#define ZOIDFS_PIPELINE_SIZE (char*)"ZOIDFS_PIPELINE_SIZE"

/* vampir hints */
#define ZOIDFS_ATOMIC_APPEND (char*)"ZOIDFS_ATOMIC_APPEND"
#define ZOIDFS_ATOMIC_APPEND_OFFSET (char*)"ZOIDFS_ATOMIC_APPEND_OFFSET"
#define ZOIDFS_ATOMIC_APPEND_OFFSET_MAX_BYTES 22 
#define ZOIDFS_ATOMIC_APPEND_SEEK (char*)"ZOIDFS_ATOMIC_APPEND_SEEK"
#define ZOIDFS_NONBLOCK_SERVER_IO (char*)"ZOIDFS_NONBLOCK_SERVER_IO"

/* hint limits */
#define ZOIDFS_HINT_MAX_HINTS 128
#define ZOIDFS_HINT_MAX_KEY_SIZE 128
#define ZOIDFS_HINT_MAX_VALUE_SIZE 128

/* hint API */
int zoidfs_hint_get_max_size();
int zoidfs_hint_create(zoidfs_op_hint_t * hint);
int zoidfs_hint_set(zoidfs_op_hint_t hint, char * key, char * value, int bin_length);
int zoidfs_hint_set_raw(zoidfs_op_hint_t hint, char * key, char * value, int valuelen);
int zoidfs_hint_delete(zoidfs_op_hint_t hint, char * key);
int zoidfs_hint_delete_all(zoidfs_op_hint_t hint);
int zoidfs_hint_get(zoidfs_op_hint_t hint, char * key, int valuelen, char * value, int * flag);
void * zoidfs_hint_get_raw(zoidfs_op_hint_t hint, char * key, int valuelen, int * flag);
int zoidfs_hint_get_valuelen(zoidfs_op_hint_t hint, char * key, int * valuelen, int * flag);
int zoidfs_hint_get_nkeys(zoidfs_op_hint_t hint, int * nkeys);
int zoidfs_hint_get_nthkey(zoidfs_op_hint_t hint, int n, char * key);
void * zoidfs_hint_get_nthkey_raw(zoidfs_op_hint_t hint, int n);
int zoidfs_hint_get_nthkeylen(zoidfs_op_hint_t hint, int n, int * keylen);
int zoidfs_hint_dup(zoidfs_op_hint_t oldhint, zoidfs_op_hint_t * newhint);
int zoidfs_hint_copy(zoidfs_op_hint_t * oldhint, zoidfs_op_hint_t * newhint);
int zoidfs_hint_free(zoidfs_op_hint_t * hint);

#ifdef __cplusplus
    } /* extern */
    } /* namespace hints */
} /* namespace zoidfs */
#endif /* __cplusplus */

#endif /* __SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C__ */
