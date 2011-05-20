#include "iofwd/rpcfrontend/IOFSLRPCResizeRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      const IOFSLRPCResizeRequest::ReqParam & IOFSLRPCResizeRequest::decodeParam ()
      {
         param_.handle = &inStruct.handle;
         param_.op_hint = &op_hint_;
         return param_;
      }
      void IOFSLRPCResizeRequest::reply(const CBType & cb)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK);
          outStruct.returnCode = getReturnCode();
          encode(cb);
      }
      IOFSLRPCResizeRequest::~IOFSLRPCResizeRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }
   }
}
