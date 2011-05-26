#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ReadLinkRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"

#include "encoder/EncoderString.hh"
#include "common/rpc/CommonRequest.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      //typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;

      class IOFSLRPCReadLinkRequest :
          public RPCSimpleRequest<common::RPCReadLinkRequest, common::RPCReadLinkResponse>,
          public ReadLinkRequest
      {
          public:
              IOFSLRPCReadLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCReadLinkRequest, common::RPCReadLinkResponse>(in, out),
                  ReadLinkRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCReadLinkRequest();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, const char * buffer_,
                                  size_t buffer_length_);
                      
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
