#ifndef IOFWD_RPCFRONTEND_IOFSLRPCNOTIMPLEMENTED_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCNOTIMPLEMENTED_HH

/* NO CPP FILE */

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/NotImplementedRequest.hh"
#include "iofwd/frontend/rpc/RPCSimpleRequest.hh"
#include "iofwdevent/CBType.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      ENCODERSTRUCT (RPCNotImpliin, );
      ENCODERSTRUCT (RPCNotImpliOut, ((int)(returnCode)))
      class IOFSLRPCNotImplementedRequest :
          public RPCSimpleRequest<RPCNotImpliin, RPCNotImpliOut>,
          public NotImplementedRequest
      {
          public:
              IOFSLRPCNotImplementedRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<RPCNotImpliin, RPCNotImpliOut>(in, out),
                  NotImplementedRequest(opid)
              {
              }
            
              ~IOFSLRPCNotImplementedRequest();
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
