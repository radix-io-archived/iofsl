#include "iofwd/rpcfrontend/IOFSLRPCGetAttrRequest.hh"
#include "iofwdutil/tools.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      RPC_GENPROCESS (IOFSLRPCGetAttrRequest, ((&)(handle))
                                              ((&)(attr)),
                                              (()(returnCode))
                                              ((*)(attr_enc_)))



      void IOFSLRPCGetAttrRequest::reply(const CBType & UNUSED(cb),
              const zoidfs::zoidfs_attr_t * attr)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || attr);

          /* store ptr to the attr */
          attr_enc_ = const_cast<zoidfs::zoidfs_attr_t *>(attr);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCGetAttrRequest::~IOFSLRPCGetAttrRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCGetAttrRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCGetAttrRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
