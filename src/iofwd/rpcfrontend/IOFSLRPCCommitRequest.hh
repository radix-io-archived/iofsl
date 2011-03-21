#ifndef IOFWD_RPCFRONTEND_IOFSLRPCCOMMITREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCCOMMITREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/CommitRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
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
          public IOFSLRPCRequest,
          public CommitRequest
      {
          public:
              IOFSLRPCCommitRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  CommitRequest(opid)
              {
              }

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb);
              ~IOFSLRPCCommitRequest();
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCCommitDec dec_struct;
              IOFSLRPCCommitEnc enc_struct;

              zoidfs::zoidfs_op_hint_t op_hint_;
      };
   }
}
#endif
