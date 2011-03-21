#include "iofwd/rpcfrontend/IOFSLRPCCommitRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      /* Generate Encode/Decode Functions */
      RPC_GENPROCESS (IOFSLRPCCommitRequest, ((&)(handle)),
                                             (()(returnCode)))

      const IOFSLRPCCommitRequest::ReqParam & IOFSLRPCCommitRequest::decodeParam ()
      {
         param_.handle = &dec_struct.handle;
         param_.op_hint = &op_hint_;
         return param_;
      }
      void IOFSLRPCCommitRequest::reply(const CBType & UNUSED(cb))
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }
      IOFSLRPCCommitRequest::~IOFSLRPCCommitRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCCommitRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCCommitRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }
   }
}
