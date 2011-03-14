#ifndef IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/LookupRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

class IOFSLRPCLookupRequest :
    public IOFSLRPCRequest,
    public LookupRequest
{
    public:
        IOFSLRPCLookupRequest(int opid,
                iofwdevent::ZeroCopyInputStream * in,
                iofwdevent::ZeroCopyOutputStream * out) :
            IOFSLRPCRequest(in, out),
            LookupRequest(opid),
            handle_enc_(NULL)
        {
        }
      
        virtual ~IOFSLRPCLookupRequest();

        /* encode and decode helpers for RPC data */
        virtual void decode();
        virtual void encode();

        /* request processing */
        virtual const ReqParam & decodeParam();
        virtual void reply(const CBType & cb,
                const zoidfs::zoidfs_handle_t * handle);
    
    protected:
        /* data size helpers for this request */ 
        virtual size_t rpcEncodedInputDataSize(); 
        virtual size_t rpcEncodedOutputDataSize();

        ReqParam param_;
        FileInfo info_;
        zoidfs::zoidfs_handle_t * handle_enc_;
        zoidfs::zoidfs_op_hint_t op_hint_;
};

   }
}

#endif
