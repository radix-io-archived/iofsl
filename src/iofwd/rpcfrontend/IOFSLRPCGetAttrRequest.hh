#ifndef IOFWD_RPCFRONTEND_IOFSLRPCGETATTRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCGETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "iofwd/GetAttrRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "encoder/Util.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
typedef zoidfs::zoidfs_attr_t zoidfs_attr_t;
ENCODERSTRUCT (IOFSLRPCGetAttrDec, ((zoidfs_handle_t)(handle))
                                   ((zoidfs_attr_t)(attr)))
ENCODERSTRUCT (IOFSLRPCGetAttrEnc, ((int)(returnCode))
                                    ((zoidfs_attr_t)(attr_enc_)))

class IOFSLRPCGetAttrRequest :
    public IOFSLRPCRequest,
    public GetAttrRequest
{
    public:
        IOFSLRPCGetAttrRequest(int opid,
                iofwdevent::ZeroCopyInputStream * in,
                iofwdevent::ZeroCopyOutputStream * out) :
            IOFSLRPCRequest(in, out),
            GetAttrRequest(opid),
            attr_enc_(NULL)
        {
        }
      
        virtual ~IOFSLRPCGetAttrRequest();

        /* encode and decode helpers for RPC data */
        virtual void decode();
        virtual void encode();

        /* request processing */
        virtual const ReqParam & decodeParam();
        virtual void reply(const CBType & cb,
                const zoidfs::zoidfs_attr_t * attr);
    
    protected:
        /* data size helpers for this request */ 
        virtual size_t rpcEncodedInputDataSize(); 
        virtual size_t rpcEncodedOutputDataSize();
        IOFSLRPCGetAttrDec dec_struct;
        IOFSLRPCGetAttrEnc enc_struct;
        ReqParam param_;
        zoidfs::zoidfs_op_hint_t op_hint_;
        zoidfs::zoidfs_attr_t * attr_enc_;
};

   }
}

#endif
