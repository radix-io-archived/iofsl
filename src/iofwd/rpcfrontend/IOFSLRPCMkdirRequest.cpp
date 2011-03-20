#include "iofwd/rpcfrontend/IOFSLRPCMkdirRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      RPC_GENPROCESS (IOFSLRPCMkdirRequest,(()(full_path)) 
                                          ((&)(parent_handle))
                                          (()(component_name))              
                                          ((&)(sattr)) ,
                                          (()(returnCode))
                                          ((*)(parent_hint)))
                                            

      const IOFSLRPCMkdirRequest::ReqParam & IOFSLRPCMkdirRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          if (dec_struct.full_path.value.size() > 0)
          {
            param_.full_path = const_cast<char *>(dec_struct.full_path.value.c_str());
            param_.component_name = 0;
            param_.parent_handle = 0; 
          }
          else
          {
            param_.full_path = 0;
            param_.component_name = const_cast<char *>(dec_struct.component_name.value.c_str());
            param_.parent_handle = &dec_struct.parent_handle; 
          }
          param_.sattr = &dec_struct.sattr;
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCMkdirRequest::reply(const CBType & cb, 
                              const zoidfs::zoidfs_cache_hint_t * parent_hint_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || parent_hint_);

          /* store ptr to the attr */
          parent_hint = const_cast<zoidfs::zoidfs_cache_hint_t *>(parent_hint_);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCMkdirRequest::~IOFSLRPCMkdirRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCMkdirRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCMkdirRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
