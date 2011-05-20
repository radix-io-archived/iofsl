#include "iofwd/rpcfrontend/IOFSLRPCReadLinkRequest.hh"
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
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCReadLinkRequest::reply(const CBType & cb, const char * buffer_,
                                          size_t buffer_length_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || buffer_ || buffer_length_);
          outStruct.buffer.value.assign(buffer_);
          outStruct.buffer_length = buffer_length_;
          /* encode */
          encode(cb);
      }

      IOFSLRPCReadLinkRequest::~IOFSLRPCReadLinkRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

   }
}
