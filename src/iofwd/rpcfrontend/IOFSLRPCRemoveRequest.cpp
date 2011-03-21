#include "iofwd/rpcfrontend/IOFSLRPCRemoveRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      RPC_GENPROCESS (IOFSLRPCRemoveRequest,(()(full_path)) 
                                          ((&)(parent_handle))
                                          (()(component_name)),
                                          (()(returnCode))
                                          ((*)(parent_hint)))
                                            

      const IOFSLRPCRemoveRequest::ReqParam & IOFSLRPCRemoveRequest::decodeParam() 
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
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCRemoveRequest::reply(const CBType & cb, 
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

      IOFSLRPCRemoveRequest::~IOFSLRPCRemoveRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCRemoveRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCRemoveRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
