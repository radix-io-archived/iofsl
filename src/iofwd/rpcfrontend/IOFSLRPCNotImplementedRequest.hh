#ifndef IOFWD_RPCFRONTEND_IOFSLRPCNOTIMPLEMENTED_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCNOTIMPLEMENTED_HH

/* NO CPP FILE */

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/NotImplementedRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "iofwdevent/CBType.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCNotImplementedRequest :
          public IOFSLRPCRequest,
          public NotImplementedRequest
      {
          public:
              IOFSLRPCNotImplementedRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  NotImplementedRequest(opid)
              {
              }
            
              ~IOFSLRPCNotImplementedRequest();

              /* encode and decode helpers for RPC data */
              void decode() {};
              void encode() { process(enc_, getReturnCode()); };

              /* request processing */
              const ReqParam & decodeParam() {return param_;};
              void reply (const CBType & cb);
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize()  { return 0; }; 
              virtual size_t rpcEncodedOutputDataSize() { return 0; };

              ReqParam param_;
      };
   }
}

#endif
