#ifndef ZOIDFS_UTIL_ZOIDFSOPHINT_HH
#define ZOIDFS_UTIL_ZOIDFSOPHINT_HH

#include <string>

#include "zoidfs/hints/zoidfs-hints.h"

namespace zoidfs
{
    namespace util
    {

class ZoidFSOpHint
{
    public:
        ZoidFSOpHint();
        ~ZoidFSOpHint();

        int getHint(std::string hint_key,
                size_t hint_value_len,
                char * hint_value,
                int * found);

        int getHintValueLen(std::string hint_key,
                int * hint_value_len,
                int * found);

        int deleteHint(std::string hint_key);

        int setHint(std::string hint_key,
                char * hint_value,
                int hint_value_len);

        zoidfs::zoidfs_op_hint_t * operator() ()
        {
            return &hint_;
        }

    protected:
        zoidfs::zoidfs_op_hint_t hint_;
};

    }
}

#endif
