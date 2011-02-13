#include "IOFWDRequest.hh"

#include "zoidfs/zoidfs.h"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   IOFWDRequest::IOFWDRequest (RequestTracker * t)
      : returncode_ (ZFS_OK),
        completionstatus_ (ZFS_COMP_DONE),
        tracker_ (t)
   {
   }

   //========================================================================
}
