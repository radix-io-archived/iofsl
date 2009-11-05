#include "zoidfs/util/ZoidFSHints.hh"
#include "zoidfs/hints/zoidfs-hints.h"

namespace zoidfs
{
    namespace util
    {
        zoidfs::zoidfs_op_hint_t * ZoidFSHintInit(int size)
        {
            return zoidfs::zoidfs_hint_init(size);
        }

        int ZoidFSHintAdd(zoidfs::zoidfs_op_hint_t ** op_hint, char * key, char * value, int value_len, int flags)
        {
            return zoidfs::zoidfs_hint_add(op_hint, key, value, value_len, flags);
        }

        int ZoidFSHintRemove(zoidfs::zoidfs_op_hint_t ** op_hint, char * key, int flags)
        {
            return zoidfs::zoidfs_hint_remove(op_hint, key, flags);
        }

        char * ZoidFSHintGet(zoidfs::zoidfs_op_hint_t ** op_hint, char * key)
        {
            return zoidfs::zoidfs_hint_get(op_hint, key);
        }

        int ZoidFSHintDestroy(zoidfs::zoidfs_op_hint_t ** op_hint)
        {
            return zoidfs::zoidfs_hint_destroy(op_hint);
        }

        zoidfs::zoidfs_op_hint_t * ZoidFSHintPop(zoidfs::zoidfs_op_hint_t ** op_hint)
        {
            return zoidfs::zoidfs_hint_pop(op_hint);
        }

        int ZoidFSHintPrint(zoidfs::zoidfs_op_hint_t ** op_hint)
        {
            return zoidfs::zoidfs_hint_print(op_hint);
        }

        int ZoidFSHintNumElements(zoidfs::zoidfs_op_hint_t ** op_hint)
        {
            return zoidfs::zoidfs_hint_num_elements(op_hint);
        }

        zoidfs::zoidfs_op_hint_t * ZoidFSHintIndex(zoidfs::zoidfs_op_hint_t ** op_hint, int index)
        {
            return zoidfs::zoidfs_hint_index(op_hint, index);
        }
        char * ZoidFSHintMakeKey(int key_len)
        {
            return zoidfs::zoidfs_hint_make_key(key_len);
        }

        int ZoidFSHintRmKey(char * key)
        {
            return zoidfs::zoidfs_hint_rm_key(key);
        }

        char * ZoidFSHintMakeValue(int value_len)
        {
            return zoidfs::zoidfs_hint_make_value(value_len);
        }

        int ZoidFSHintValueKey(char * value)
        {
            return zoidfs::zoidfs_hint_rm_value(value);
        }
    }
}
