#include "iofwd/frontend/rpc/IOFSLRPCRemoveRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      const IOFSLRPCRemoveRequest::ReqParam & IOFSLRPCRemoveRequest::decodeParam() 
      {
          /* decode the rpc input params */
         if(inStruct.info.full_path.value.c_str()[0])
         {
            param_.full_path = (char *)inStruct.info.full_path.value.c_str();
            param_.component_name = 0;
            param_.parent_handle = 0;
         }
         else
         {
            param_.full_path = 0;
            param_.parent_handle = &inStruct.info.handle;
            param_.component_name = (char *)inStruct.info.component.value.c_str();
         }
         param_.op_hint = NULL;

         return param_;
      }

      void IOFSLRPCRemoveRequest::reply(const CBType & cb, 
                              const zoidfs::zoidfs_cache_hint_t * parent_hint_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK);

          /* store ptr to the attr */
          outStruct.parent_hint = *parent_hint_;
          outStruct.returnCode = getReturnCode();

          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          /* @TODO: Remove this later */
          param_.op_hint = &op_hint_;

          encode(cb);
      }

      IOFSLRPCRemoveRequest::~IOFSLRPCRemoveRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }
   }
}
