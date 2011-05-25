#ifndef IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/LookupRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "RPCSimpleRequest.hh"
#include "encoder/EncoderCommon.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
class IOFSLRPCLookupRequest :
    public RPCSimpleRequest<encoder::LookupRequest, encoder::LookupResponse>,
    public LookupRequest
{
    public:
        IOFSLRPCLookupRequest(int opid,
                iofwdevent::ZeroCopyInputStream * in,
                iofwdevent::ZeroCopyOutputStream * out) :
            RPCSimpleRequest<encoder::LookupRequest, encoder::LookupResponse>(in, out),
            LookupRequest(opid)
        {
        }
      
        virtual ~IOFSLRPCLookupRequest();

        /* request processing */
        virtual const ReqParam & decodeParam();
        virtual void reply(const CBType & cb,
                const zoidfs::zoidfs_handle_t * handle);
    
    protected:
        ReqParam param_;
};

   }
}

#endif
