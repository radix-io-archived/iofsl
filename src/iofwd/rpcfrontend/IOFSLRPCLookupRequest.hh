#ifndef IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCLOOKUPREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/LookupRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "RPCSimpleRequest.hh"
#include "zoidfs/util/FileSpecHelper.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
/* Input encoder struct */
typedef encoder::FileSpecHelper FileSpecHelper;
typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
ENCODERSTRUCT(RPCLookupIn,  ((FileSpecHelper)(info)))

/* Output Encoder Struct */
typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
ENCODERSTRUCT(RPCLookupOut, ((int)(returnCode))
                            ((zoidfs_handle_t)(handle)))


class IOFSLRPCLookupRequest :
    public RPCSimpleRequest<RPCLookupIn, RPCLookupOut>,
    public LookupRequest
{
    public:
        IOFSLRPCLookupRequest(int opid,
                iofwdevent::ZeroCopyInputStream * in,
                iofwdevent::ZeroCopyOutputStream * out) :
            RPCSimpleRequest<RPCLookupIn, RPCLookupOut>(in, out),
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
