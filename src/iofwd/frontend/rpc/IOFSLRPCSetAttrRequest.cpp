#include "iofwd/frontend/rpc/IOFSLRPCSetAttrRequest.hh"
#include "iofwdutil/tools.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      const IOFSLRPCSetAttrRequest::ReqParam & IOFSLRPCSetAttrRequest::decodeParam() 
      { 
          param_.handle = &inStruct.handle ; 
          param_.attr = &inStruct.attr; 
          param_.sattr = &inStruct.sattr;
          param_.op_hint = NULL; 
          return param_; 
      }
      void IOFSLRPCSetAttrRequest::reply(const CBType & cb,
              const zoidfs::zoidfs_attr_t * attr)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK);

          /* store ptr to the attr */
          outStruct.attr_enc = (*attr);
          outStruct.returnCode = getReturnCode();

          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          /* @TODO: Remove this later */
          param_.op_hint = &op_hint_;

          /* encode */
          encode(cb);
      }

      IOFSLRPCSetAttrRequest::~IOFSLRPCSetAttrRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }
   }
}
