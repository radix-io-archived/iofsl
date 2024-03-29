#ifndef IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/LookupRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCRequest.hh"
#include "RPCSimpleRequest.hh"
#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
class IOFSLRPCLookupRequest :
    public RPCSimpleRequest<common::RPCLookupRequest, common::RPCLookupResponse>,
    public LookupRequest
{
    public:
        IOFSLRPCLookupRequest(int opid,
                iofwdevent::ZeroCopyInputStream * in,
                iofwdevent::ZeroCopyOutputStream * out) :
            RPCSimpleRequest<common::RPCLookupRequest, common::RPCLookupResponse>(in, out),
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
