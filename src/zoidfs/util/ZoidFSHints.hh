#ifndef __SRC_ZOIDFS_UTIL_ZOIDFS_HINTS_HH__
#define __SRC_ZOIDFS_UTIL_ZOIDFS_HINTS_HH__

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/hints/zoidfs-hints.h"

namespace zoidfs
{
    namespace util
    {
        zoidfs::zoidfs_op_hint_t * ZoidFSHintInit(int size);
        int ZoidFSHintAdd(zoidfs::zoidfs_op_hint_t ** op_hint, char * key, char * value, int value_len, int flags);
        int ZoidFSHintRemove(zoidfs::zoidfs_op_hint_t ** op_hint, char * key);
        char * ZoidFSHintGet(zoidfs::zoidfs_op_hint_t ** op_hint, char * key);
        int ZoidFSHintDestroy(zoidfs::zoidfs_op_hint_t ** op_hint);
        zoidfs::zoidfs_op_hint_t * ZoidFSHintPop(zoidfs::zoidfs_op_hint_t ** op_hint);
        int ZoidFSHintPrint(zoidfs::zoidfs_op_hint_t ** op_hint);
        int ZoidFSHintNumElements(zoidfs::zoidfs_op_hint_t ** op_hint);
        zoidfs::zoidfs_op_hint_t * ZoidFSHintIndex(zoidfs::zoidfs_op_hint_t ** op_hint, int index);
        char * ZoidFSHintMakeKey(int key_len);
        int ZoidFSHintRmKey(char * key);
        char * ZoidFSHintMakeValue(int value_len);
        int ZoidFSHintValueKey(char * value);
    }
}
#endif
