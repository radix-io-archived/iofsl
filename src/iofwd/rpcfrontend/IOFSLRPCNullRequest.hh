#ifndef IOFWD_RPCFRONTEND_IOFSLRPCNULLREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCNULLREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/NullRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "iofwdevent/CBType.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      typedef struct 
      {} ReqParam;
      
      class IOFSLRPCNullRequest :
          public IOFSLRPCRequest,
          public NullRequest
      {
          public:
              IOFSLRPCNullRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  NullRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCNullRequest();

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
