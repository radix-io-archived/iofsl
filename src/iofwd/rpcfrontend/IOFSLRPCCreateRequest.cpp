#include "iofwd/rpcfrontend/IOFSLRPCCreateRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      const IOFSLRPCCreateRequest::ReqParam & IOFSLRPCCreateRequest::decodeParam() 
      { 
            /* decode the rpc input params */
          decode();
          fprintf(stderr, "IOFSLRPCCreateRequest:%s:%i\n", __func__, __LINE__);
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
          param_.attr = &inStruct.attr; 
          param_.op_hint = NULL;
//          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCCreateRequest::reply(const CBType & UNUSED(cb),
              const zoidfs_handle_t * handle_, int created_)
      {
          fprintf(stderr, "IOFSLRPCCreateRequest:%s:%i\n", __func__, __LINE__);
          /* verify the args are OK */
//          ASSERT(getReturnCode() != zoidfs::ZFS_OK || handle_);

          /* Store handle/return code information for output */
          outStruct.handle = (*handle_);
          outStruct.returnCode = getReturnCode();
          outStruct.created = created_;

          /* encode */
          encode();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCCreateRequest::~IOFSLRPCCreateRequest()
      {
         //zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

   }
}
