#include "iofwd/rpcfrontend/IOFSLRPCReadLinkRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      RPC_GENPROCESS (IOFSLRPCReadLinkRequest,((&)(handle))
                                              (()(buffer_length)) ,
                                              (()(returnCode))
                                              (()(buffer))
                                              (()(buffer_length)))
                                            

      const IOFSLRPCReadLinkRequest::ReqParam & IOFSLRPCReadLinkRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          param_.handle = &dec_struct.handle; 
          param_.buffer_length = dec_struct.buffer_length;
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCReadLinkRequest::reply(const CBType & cb, const char * buffer_,
                                          size_t buffer_length_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || buffer_ || buffer_length_);

          buffer.value.assign(buffer_);
          buffer_length = buffer_length_;

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCReadLinkRequest::~IOFSLRPCReadLinkRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCReadLinkRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCReadLinkRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
