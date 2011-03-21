#include "iofwd/rpcfrontend/IOFSLRPCSetAttrRequest.hh"
#include "iofwdutil/tools.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      RPC_GENPROCESS (IOFSLRPCSetAttrRequest, ((&)(handle))
                                              ((&)(attr))
                                              ((&)(sattr)),
                                              (()(returnCode))
                                              ((*)(attr)))


      const IOFSLRPCSetAttrRequest::ReqParam & IOFSLRPCSetAttrRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          param_.handle = &dec_struct.handle ; 
          param_.attr = &dec_struct.attr; 
          param_.sattr = &dec_struct.sattr;

          param_.op_hint = &op_hint_; 
          return param_; 
      }
      void IOFSLRPCSetAttrRequest::reply(const CBType & UNUSED(cb),
              const zoidfs::zoidfs_attr_t * attr)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || attr);

          /* store ptr to the attr */
          attr = const_cast<zoidfs::zoidfs_attr_t *>(attr);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCSetAttrRequest::~IOFSLRPCSetAttrRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCSetAttrRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCSetAttrRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
