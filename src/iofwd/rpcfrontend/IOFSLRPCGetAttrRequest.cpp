#include "iofwd/rpcfrontend/IOFSLRPCGetAttrRequest.hh"
#include "iofwdutil/tools.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      const IOFSLRPCGetAttrRequest::ReqParam & IOFSLRPCGetAttrRequest::decodeParam() 
      { 
          param_.handle = &inStruct.handle ; 
          param_.attr = &inStruct.attr ; 
          param_.op_hint = &op_hint_; 
          return param_; 
      }
      void IOFSLRPCGetAttrRequest::reply(const CBType & cb,
                                         const zoidfs::zoidfs_attr_t * attr)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || attr);

          /* store ptr to the attr */
          outStruct.attr_enc = (*attr);

          /* encode */
          encode(cb);

      }

      IOFSLRPCGetAttrRequest::~IOFSLRPCGetAttrRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

   }
}
