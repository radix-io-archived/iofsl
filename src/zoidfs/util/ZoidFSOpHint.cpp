#include "zoidfs/util/ZoidFSOpHint.hh"

namespace zoidfs
{
    namespace util
    {
        ZoidFSOpHint::ZoidFSOpHint()
        {
            zoidfs::hints::zoidfs_hint_create(&hint_);
        }

        ZoidFSOpHint::~ZoidFSOpHint()
        {
            zoidfs::hints::zoidfs_hint_free(&hint_);
        }

        int ZoidFSOpHint::getHint(std::string hint_key,
                size_t hint_value_len,
                char * hint_value,
                int * found)
        {
            return zoidfs::hints::zoidfs_hint_get(hint_, (char *)hint_key.c_str(), 
                    hint_value_len, hint_value, found);
        }

        int ZoidFSOpHint::getHintValueLen(std::string hint_key,
                int * hint_value_len,
                int * found)
        {
            return zoidfs::hints::zoidfs_hint_get_valuelen(hint_,
                    (char *)hint_key.c_str(), hint_value_len, found);
        }

        int ZoidFSOpHint::deleteHint(std::string hint_key)
        {
            return zoidfs::hints::zoidfs_hint_delete(hint_,
                    (char *)hint_key.c_str());
        }

        int ZoidFSOpHint::setHint(std::string hint_key,
                char * hint_value,
                int hint_value_len)
        {
            return zoidfs::hints::zoidfs_hint_set(hint_,
                    (char *)hint_key.c_str(), hint_value, hint_value_len);
        }
    }
}
