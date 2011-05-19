#include "iofwd/rpcfrontend/IOFSLRPCCommitRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      const IOFSLRPCCommitRequest::ReqParam & IOFSLRPCCommitRequest::decodeParam ()
      {
         param_.handle = &inStruct.handle;
         param_.op_hint = &op_hint_;
         return param_;
      }

      void IOFSLRPCCommitRequest::reply(const CBType & cb)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);
          /* Store handle/return code information for output */
          outStruct.returnCode = getReturnCode();

          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          param_.op_hint = &op_hint_;
          
          encode(cb);
      }

      IOFSLRPCCommitRequest::~IOFSLRPCCommitRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }
   }
}
