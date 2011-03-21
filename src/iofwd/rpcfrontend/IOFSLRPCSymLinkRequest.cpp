#include "iofwd/rpcfrontend/IOFSLRPCSymLinkRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
        RPC_GENPROCESS (IOFSLRPCSymLinkRequest, (()(from_full_path)) 
                                        ((&)(from_parent_handle))
                                        (()(from_component_name))              
                                        (()(to_full_path))
                                        ((&)(to_parent_handle))
                                        (()(to_component_name))
                                        ((&)(sattr)),
                                        (()(returnCode))
                                        ((*)(from_parent_hint))
                                        ((*)(to_parent_hint)))

      const IOFSLRPCSymLinkRequest::ReqParam & IOFSLRPCSymLinkRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          if (dec_struct.from_full_path.value.size() > 0)
          {
            param_.from_full_path = const_cast<char *>(dec_struct.from_full_path.value.c_str());
            param_.from_component_name = 0;
            param_.from_parent_handle = 0; 
          }
          else
          {
            param_.from_full_path = 0;
            param_.from_component_name = const_cast<char *>(dec_struct.from_component_name.value.c_str());
            param_.from_parent_handle = &dec_struct.from_parent_handle; 
          }

          if (dec_struct.to_full_path.value.size() > 0)
          {
            param_.to_full_path = const_cast<char *>(dec_struct.to_full_path.value.c_str());
            param_.to_component_name = 0;
            param_.to_parent_handle = 0;
          }
          else
          {
            param_.to_full_path = 0;
            param_.to_component_name = const_cast<char *>(dec_struct.to_component_name.value.c_str());
            param_.to_parent_handle = &dec_struct.to_parent_handle;
          }
          param_.sattr = &dec_struct.sattr;
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCSymLinkRequest::reply(const CBType & cb, 
                                      const zoidfs::zoidfs_cache_hint_t *
                                        from_parent_hint_, 
                                      const zoidfs::zoidfs_cache_hint_t * 
                                        to_parent_hint_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || from_parent_hint_ || to_parent_hint_);

          /* store ptr to the attr */
          to_parent_hint = const_cast<zoidfs::zoidfs_cache_hint_t *>(to_parent_hint_);
          from_parent_hint = const_cast<zoidfs::zoidfs_cache_hint_t *>(from_parent_hint_);

          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCSymLinkRequest::~IOFSLRPCSymLinkRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCSymLinkRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCSymLinkRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
