#ifndef IOFWD_RPCFRONTEND_IOFSLRPCCOMMITREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCCOMMITREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/CommitRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "encoder/EncoderWrappers.hh"
using namespace encoder;

namespace iofwd
{
   namespace rpcfrontend
   {

      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;

      ENCODERSTRUCT (IOFSLRPCCommitDec, ((zoidfs_handle_t)(handle)))

      ENCODERSTRUCT (IOFSLRPCCommitEnc, ((int)(returnCode)))

      class IOFSLRPCCommitRequest :
          public RPCSimpleRequest<IOFSLRPCCommitDec, IOFSLRPCCommitEnc>,
          public CommitRequest
      {
          public:
              IOFSLRPCCommitRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<IOFSLRPCCommitDec, IOFSLRPCCommitEnc>(in, out),
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
