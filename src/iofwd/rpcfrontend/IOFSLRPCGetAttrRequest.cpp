#include "iofwd/rpcfrontend/IOFSLRPCGetAttrRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

void IOFSLRPCGetAttrRequest::decode()
{
    process(dec_, handle_);
    process(dec_, attr_);
    zoidfs::hints::zoidfs_hint_create(&op_hint_);
    decodeOpHint(&op_hint_);
}

const IOFSLRPCGetAttrRequest::ReqParam & IOFSLRPCGetAttrRequest::decodeParam()
{
    /* decode the rpc input params */
    decodeRPCInput();

    /* setup the req param */
    param_.handle = &handle_;
    param_.attr = &attr_;
    param_.op_hint = &op_hint_;

    /* return a ref to the req param */
    return param_;
}

void IOFSLRPCGetAttrRequest::encode()
{
    /* process the output */
    process(enc_, getReturnCode());
    process(enc_, const_cast<const zoidfs::zoidfs_attr_t &>(*attr_enc_));
}

void IOFSLRPCGetAttrRequest::reply(const CBType & UNUSED(cb),
        const zoidfs::zoidfs_attr_t * attr)
{
    /* verify the args are OK */
    ASSERT(getReturnCode() != zoidfs::ZFS_OK || attr);

    /* store ptr to the attr */
    attr_enc_ = const_cast<zoidfs::zoidfs_attr_t *>(attr);

    /* encode */
    encodeRPCOutput();

    /* invoke the callback */
    //cb();
}

IOFSLRPCGetAttrRequest::~IOFSLRPCGetAttrRequest()
{
   zoidfs::hints::zoidfs_hint_free(&op_hint_);
}

size_t IOFSLRPCGetAttrRequest::rpcEncodedInputDataSize()
{
    return 0;
}

size_t IOFSLRPCGetAttrRequest::rpcEncodedOutputDataSize()
{
    return 0;
}

   }
}
