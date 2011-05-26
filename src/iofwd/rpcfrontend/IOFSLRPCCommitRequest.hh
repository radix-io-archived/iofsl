#ifndef IOFWD_RPCFRONTEND_IOFSLRPCCOMMITREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCCOMMITREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/CommitRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "encoder/EncoderWrappers.hh"
#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCCommitRequest :
          public RPCSimpleRequest<common::CommitRequest, common::CommitResponse>,
          public CommitRequest
      {
          public:
              IOFSLRPCCommitRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::CommitRequest, common::CommitResponse>(in, out),
                  CommitRequest(opid)
              {
              }
              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb);
              ~IOFSLRPCCommitRequest();
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };
   }
}
#endif
