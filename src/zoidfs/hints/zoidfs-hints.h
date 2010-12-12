#ifndef SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C
#define SRC_ZOIDFS_HINTS_ZOIDFS_HINTS_C

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


/* new hint API */

/* zoidfs hints are modeled on the MPI info interface */

/* TODO these are currently just stubs */

int zoidfs_hint_create(zoidfs_op_hint_t * hint);
int zoidfs_hint_set(zoidfs_op_hint_t hint, char * key, char * value, int bin_length);
int zoidfs_hint_delete(zoidfs_op_hint_t hint, char * key);
int zoidfs_hint_get(zoidfs_op_hint_t hint, char * key, int valuelen, char * value, int * flag);
int zoidfs_hint_get_valuelen(zoidfs_op_hint_t hint, char * key, int * valuelen, int * flag);
int zoidfs_hint_get_nkeys(zoidfs_op_hint_t hint, int * nkeys);
int zoidfs_hint_get_nthkey(zoidfs_op_hint_t hint, int n, char * key);
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
