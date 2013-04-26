#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREMOVEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREMOVEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/RemoveRequest.hh"
#include "iofwd/frontend/rpc/RPCSimpleRequest.hh"
#include "common/rpc/CommonRequest.hh"

using namespace encoder;

namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCRemoveRequest :
          public RPCSimpleRequest<common::RPCRemoveRequest, common::RPCRemoveResponse>,
          public RemoveRequest
      {
          public:
              IOFSLRPCRemoveRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCRemoveRequest, common::RPCRemoveResponse>(in, out),
                  RemoveRequest(opid)
              {
              }

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                              const zoidfs::zoidfs_cache_hint_t * parent_hint_);
              ~IOFSLRPCRemoveRequest();
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };
   }
}
#endif
