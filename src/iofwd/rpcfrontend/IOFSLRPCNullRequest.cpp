#include "iofwd/rpcfrontend/IOFSLRPCNullRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
        void IOFSLRPCNullRequest::reply (const CBType & cb)
        {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);
          /* encode */
          encodeRPCOutput();
        }
      IOFSLRPCNullRequest::~IOFSLRPCNullRequest()
      {
      }

   }
}