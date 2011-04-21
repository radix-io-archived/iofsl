#include "iofwd/rpcfrontend/IOFSLRPCLookupRequest.hh"
#include "iofwdutil/tools.hh"
#include <cstdio>
namespace iofwd
{
   namespace rpcfrontend
   {

const IOFSLRPCLookupRequest::ReqParam & IOFSLRPCLookupRequest::decodeParam()
{
    /* decode the rpc input params */
    decode();
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
//   param_.op_hint = inStruct.hint;

   return param_;
}

void IOFSLRPCLookupRequest::reply(const CBType & cb, const
        zoidfs::zoidfs_handle_t * handle)
{
    /* verify the args are OK */
    ASSERT(getReturnCode() != zoidfs::ZFS_OK || handle);

    /* Store handle/return code information for output */
    outStruct.handle = (*handle);
    outStruct.returnCode = getReturnCode();

    /* encode */
    encode();
    zoidfs::hints::zoidfs_hint_create(&op_hint_);  
    /* @TODO: Remove this later */
    param_.op_hint = &op_hint_;
    cb(iofwdevent::CBException());
    /* invoke the callback */
    //cb();
}

IOFSLRPCLookupRequest::~IOFSLRPCLookupRequest()
{
    zoidfs::hints::zoidfs_hint_free(&op_hint_);
}

   }
}
