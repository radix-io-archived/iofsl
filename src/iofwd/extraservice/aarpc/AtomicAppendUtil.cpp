#include "iofwd/extraservice/aarpc/AtomicAppendUtil.hh"

#include <cstring>
#include <cstdlib>

namespace iofwd
{
    namespace extraservice
    {

    /* compute the hash of the zoidfs file handle */
    /* derived from the posix driver fcache */
    uint64_t AtomicAppendFileHandleHash(zoidfs::zoidfs_handle_t * h)
    {
        const unsigned char * ptr = (unsigned char *)&h->data;
        unsigned char init[sizeof(uint64_t)];
        unsigned int  i;
        unsigned int pos = 0;
       
        /* clear the hash */ 
        memset(&init, 0, sizeof(init));
       
        /* for each byte in the handle */ 
        for(i = 0; i < sizeof(h->data) ; i++)
        {
            /* XOR the handle byte with the hash */
            init[pos++] ^= *ptr++;

            /* wrap around */
            if(pos == sizeof(uint64_t))
            {
                pos = 0;
            }
        }

        /* return the hash */ 
        return *(uint64_t *)&init[0];
    }

    }
}
