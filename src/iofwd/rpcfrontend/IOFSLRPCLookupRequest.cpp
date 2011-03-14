#include "iofwd/rpcfrontend/IOFSLRPCLookupRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

void IOFSLRPCLookupRequest::decode()
{
    decodeFileSpec(info_);
    zoidfs::hints::zoidfs_hint_create(&op_hint_);
    decodeOpHint(&op_hint_);
}

const IOFSLRPCLookupRequest::ReqParam & IOFSLRPCLookupRequest::decodeParam()
{
    /* decode the rpc input params */
    decodeRPCInput();

   if(info_.full_path[0])
   {
      param_.full_path = info_.full_path;
      param_.component_name = 0;
      param_.parent_handle = 0;
   }
   else
   {
      param_.full_path = 0;
      param_.parent_handle = &info_.parent_handle ;
      param_.component_name = info_.component_name;
   }

   param_.op_hint = &op_hint_;

   return param_;
}

void IOFSLRPCLookupRequest::encode()
{
    /* process the output */
    process(enc_, getReturnCode());
    process(enc_, const_cast<const zoidfs::zoidfs_handle_t &>(*handle_enc_));
}

void IOFSLRPCLookupRequest::reply(const CBType & UNUSED(cb), const
        zoidfs::zoidfs_handle_t * handle)
{
    /* verify the args are OK */
    ASSERT(getReturnCode() != zoidfs::ZFS_OK || handle);

    /* store ptr to the attr */
    handle_enc_ = const_cast<zoidfs::zoidfs_handle_t *>(handle);

    /* encode */
    encodeRPCOutput();

    /* invoke the callback */
    //cb();
}

IOFSLRPCLookupRequest::~IOFSLRPCLookupRequest()
{
   zoidfs::hints::zoidfs_hint_free(&op_hint_);
}

size_t IOFSLRPCLookupRequest::rpcEncodedInputDataSize()
{
    return 0;
}

size_t IOFSLRPCLookupRequest::rpcEncodedOutputDataSize()
{
    return 0;
}

   }
}
