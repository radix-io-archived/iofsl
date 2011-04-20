#include "iofwd/extraservice/aarpc/AtomicAppendUtil.hh"

#include <cstring>
#include <cstdlib>

#include "zoidfs/util/zoidfs-util.hh"

namespace iofwd
{
    namespace extraservice
    {

    /* compute the hash of the zoidfs file handle */
    /* derived from the posix driver fcache */
    int64_t AtomicAppendFileHandleHash(zoidfs::zoidfs_handle_t * h)
    {
        const uint64_t * ptr = (uint64_t *)&h->data;
        unsigned int  i;
        uint64_t hval = 0;
       
        /* for each byte in the handle */ 
        for(i = 0 ; i < sizeof(h->data) / sizeof(uint64_t) ; i++)
        {
            /* XOR the handle byte with the hash */
            hval ^= *ptr++;
        }

        /* return the hash */ 
        return hval;
    }

    }
}
