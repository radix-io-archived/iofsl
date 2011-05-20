#include "iofwd/rpcfrontend/IOFSLRPCMkdirRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      const IOFSLRPCMkdirRequest::ReqParam & IOFSLRPCMkdirRequest::decodeParam() 
      { 
          if (inStruct.full_path.value.size() > 0)
          {
            param_.full_path = const_cast<char *>(inStruct.full_path.value.c_str());
            param_.component_name = 0;
            param_.parent_handle = 0; 
          }
          else
          {
            param_.full_path = 0;
            param_.component_name = const_cast<char *>(inStruct.component_name.value.c_str());
            param_.parent_handle = &inStruct.parent_handle; 
          }
          param_.sattr = &inStruct.sattr;
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCMkdirRequest::reply(const CBType & cb, 
                              const zoidfs::zoidfs_cache_hint_t * parent_hint_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || parent_hint_);

          /* store ptr to the attr */
          outStruct.parent_hint = *parent_hint_;
          outStruct.returnCode = getReturnCode();

          encode(cb);
      }

      IOFSLRPCMkdirRequest::~IOFSLRPCMkdirRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }
   }
}
