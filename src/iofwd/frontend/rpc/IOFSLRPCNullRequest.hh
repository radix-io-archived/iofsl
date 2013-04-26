#ifndef IOFWD_RPCFRONTEND_IOFSLRPCNULLREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCNULLREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/NullRequest.hh"
#include "iofwd/frontend/rpc/RPCSimpleRequest.hh"
#include "iofwdevent/CBType.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      ENCODERSTRUCT (RPCNotImpliin, );
      ENCODERSTRUCT (RPCNotImpliOut, ((int)(returnCode)))
      class IOFSLRPCNullRequest :
          public RPCSimpleRequest<RPCNotImpliin, RPCNotImpliOut>,
          public NullRequest
      {
          public:
              IOFSLRPCNullRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<RPCNotImpliin, RPCNotImpliOut>(in, out),
                  NullRequest(opid)
              {
              }
            
              ~IOFSLRPCNullRequest();
              /* request processing */
              const ReqParam & decodeParam() {return param_;};
              void reply (const CBType & cb);
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
      };
   }
}

#endif
