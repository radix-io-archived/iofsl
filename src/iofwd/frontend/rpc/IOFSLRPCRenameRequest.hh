#ifndef IOFWD_RPCFRONTEND_IOFSLRPCRENAMEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCRENAMEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/RenameRequest.hh"
#include "iofwd/frontend/rpc/RPCSimpleRequest.hh"
#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCRenameRequest :
          public RPCSimpleRequest<common::RPCRenameRequest, common::RPCRenameResponse>,
          public RenameRequest
      {
          public:
              IOFSLRPCRenameRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCRenameRequest, common::RPCRenameResponse>(in, out),
                  RenameRequest(opid)
              {
              }
              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                                  const zoidfs::zoidfs_cache_hint_t *
                                    from_parent_hint_, 
                                  const zoidfs::zoidfs_cache_hint_t * 
                                    to_parent_hint_);
          
              ~IOFSLRPCRenameRequest();
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };
   }
}
#endif
