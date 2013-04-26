#ifndef IOFWD_RPCFRONTEND_IOFSLRPCGETATTRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCGETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/GetAttrRequest.hh"
#include "iofwd/frontend/rpc/RPCSimpleRequest.hh"
#include "common/rpc/CommonRequest.hh"
#include "encoder/Util.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCGetAttrRequest :
          public RPCSimpleRequest<common::RPCGetAttrRequest, common::RPCGetAttrResponse>,
          public GetAttrRequest
      {
          public:
              IOFSLRPCGetAttrRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCGetAttrRequest, common::RPCGetAttrResponse>(in,out),
                  GetAttrRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCGetAttrRequest();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb,
                      const zoidfs::zoidfs_attr_t * attr);
          
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
