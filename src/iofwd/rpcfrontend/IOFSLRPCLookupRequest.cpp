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
    fprintf(stderr, "IOFSLRPCLookupRequest:%s:%i\n", __func__, __LINE__);
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

void IOFSLRPCLookupRequest::reply(const CBType & UNUSED(cb), const
        zoidfs::zoidfs_handle_t * handle)
{
    fprintf(stderr, "IOFSLRPCLookupRequest:%s:%i\n", __func__, __LINE__);
    /* verify the args are OK */
    ASSERT(getReturnCode() != zoidfs::ZFS_OK || handle);

    /* Store handle/return code information for output */
    outStruct.handle = (*handle);
    outStruct.returnCode = getReturnCode();

    /* encode */
    encode();

    /* invoke the callback */
    //cb();
}

IOFSLRPCLookupRequest::~IOFSLRPCLookupRequest()
{
    fprintf(stderr, "IOFSLRPCLookupRequest:%s:%i\n", __func__, __LINE__);
}

   }
}
