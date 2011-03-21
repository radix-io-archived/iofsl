#ifndef IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDUTIL_HH
#define IOFWD_EXTRASERVICE_AARPC_ATOMICAPPENDUTIL_HH

#include "zoidfs/zoidfs.h"

namespace iofwd
{
    namespace extraservice
    {
        /* produce a hash of the file handle */
        uint64_t AtomicAppendFileHandleHash(zoidfs::zoidfs_handle_t * h);
    }
}

#endif
