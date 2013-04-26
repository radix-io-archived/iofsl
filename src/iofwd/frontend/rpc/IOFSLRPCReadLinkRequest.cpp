#include "iofwd/frontend/rpc/IOFSLRPCReadLinkRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      const IOFSLRPCReadLinkRequest::ReqParam & IOFSLRPCReadLinkRequest::decodeParam() 
      { 
          param_.handle = &inStruct.handle; 
          param_.buffer_length = inStruct.buffer_length;
          param_.op_hint = NULL;
          return param_; 
      }

      void IOFSLRPCReadLinkRequest::reply(const CBType & cb, const char * buffer_,
                                          size_t buffer_length_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK );
          outStruct.returnCode = getReturnCode();
          outStruct.buffer.value.assign(buffer_, buffer_length_);
          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          /* @TODO: Remove this later */
          param_.op_hint = &op_hint_;

          /* encode */
          encode(cb);
      }

      IOFSLRPCReadLinkRequest::~IOFSLRPCReadLinkRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

   }
}
