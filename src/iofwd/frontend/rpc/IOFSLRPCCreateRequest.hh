#ifndef IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/CreateRequest.hh"
#include "iofwd/frontend/rpc/RPCSimpleRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCRequest.hh"
#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      
      class IOFSLRPCCreateRequest :
          public RPCSimpleRequest<common::RPCCreateRequest, common::RPCCreateResponse>,
          public CreateRequest
      {
          public:
              IOFSLRPCCreateRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCCreateRequest, common::RPCCreateResponse>(in, out),
                  CreateRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCCreateRequest();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb, 
                                 const zoidfs::zoidfs_handle_t * handle_, 
                                 int created_);
          
          protected:
              ReqParam param_;
      };

   }
}

#endif
