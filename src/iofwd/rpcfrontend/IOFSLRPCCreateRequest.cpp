#include "iofwd/rpcfrontend/IOFSLRPCCreateRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
        RPC_GENPROCESS (IOFSLRPCCreateRequest, ((&)(handle)) 
                                        (()(full_path))
                                        (()(component_name))              
                                        ((&)(attr)),
                                        (()(returnCode))
                                        ((*)(handle))
                                        (()(created)))

      const IOFSLRPCCreateRequest::ReqParam & IOFSLRPCCreateRequest::decodeParam() 
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
            param_.parent_handle = &dec_struct.handle;    
            param_.component_name = const_cast<char *>(dec_struct.component_name.value.c_str()); 
            param_.full_path = 0;
          }
          param_.attr = &dec_struct.attr ; 
          param_.op_hint = &op_hint_; 
          return param_; 
      }



//      RPC_GENPROCESS (IOFSLRPCCreateRequest, ((&)(handle))               
//                                              (()(full_path))
//                                              (()(component_name))
//                                              ((&)(attr)),
//                                              (()(returnCode))
//                                              ((*)(handle))
//                                              (()(created)))

      void IOFSLRPCCreateRequest::reply(const CBType & UNUSED(cb),
              const zoidfs_handle_t * handle_, int created_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || handle_);

          /* store ptr to the attr */
          handle = const_cast<zoidfs::zoidfs_handle_t *>(handle_);
          created = created_;

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCCreateRequest::~IOFSLRPCCreateRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCCreateRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCCreateRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
