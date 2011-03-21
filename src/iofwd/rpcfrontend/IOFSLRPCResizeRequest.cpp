#include "iofwd/rpcfrontend/IOFSLRPCResizeRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      /* Generate Encode/Decode Functions */
      RPC_GENPROCESS (IOFSLRPCResizeRequest, ((&)(handle))
                                             (()(size)),
                                             (()(returnCode)))

      const IOFSLRPCResizeRequest::ReqParam & IOFSLRPCResizeRequest::decodeParam ()
      {
         param_.handle = &dec_struct.handle;
         param_.op_hint = &op_hint_;
         return param_;
      }
      void IOFSLRPCResizeRequest::reply(const CBType & UNUSED(cb))
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }
      IOFSLRPCResizeRequest::~IOFSLRPCResizeRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCResizeRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCResizeRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }
   }
}
