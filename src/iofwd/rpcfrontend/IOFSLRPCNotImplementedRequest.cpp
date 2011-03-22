#include "iofwd/rpcfrontend/IOFSLRPCNotImplementedRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
        void IOFSLRPCNotImplementedRequest::reply (const CBType & cb)
        {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);
          /* encode */
          encodeRPCOutput();
        }
      IOFSLRPCNotImplementedRequest::~IOFSLRPCNotImplementedRequest()
      {
      }

   }
}
