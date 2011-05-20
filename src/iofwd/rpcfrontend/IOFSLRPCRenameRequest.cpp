#include "iofwd/rpcfrontend/IOFSLRPCRenameRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      const IOFSLRPCRenameRequest::ReqParam & IOFSLRPCRenameRequest::decodeParam() 
      { 
          if (inStruct.from_full_path.value.size() > 0)
          {
            param_.from_full_path = const_cast<char *>(inStruct.from_full_path.value.c_str());
            param_.from_component_name = 0;
            param_.from_parent_handle = 0; 
          }
          else
          {
            param_.from_full_path = 0;
            param_.from_component_name = const_cast<char *>(inStruct.from_component_name.value.c_str());
            param_.from_parent_handle = &inStruct.from_parent_handle; 
          }

          if (inStruct.to_full_path.value.size() > 0)
          {
            param_.to_full_path = const_cast<char *>(inStruct.to_full_path.value.c_str());
            param_.to_component_name = 0;
            param_.to_parent_handle = 0;
          }
          else
          {
            param_.to_full_path = 0;
            param_.to_component_name = const_cast<char *>(inStruct.to_component_name.value.c_str());
            param_.to_parent_handle = &inStruct.to_parent_handle;
          }

          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCRenameRequest::reply(const CBType & cb, 
                                      const zoidfs::zoidfs_cache_hint_t *
                                        from_parent_hint_, 
                                      const zoidfs::zoidfs_cache_hint_t * 
                                        to_parent_hint_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || from_parent_hint_ || to_parent_hint_);

          /* store ptr to the attr */
          outStruct.to_parent_hint = *to_parent_hint_;
          outStruct.from_parent_hint = *from_parent_hint_;

          /* encode */
          encode(cb);
      }

      IOFSLRPCRenameRequest::~IOFSLRPCRenameRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }
   }
}
