#ifndef IOFWD_RPCFRONTEND_IOFSLRPCLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderString.hh"
#include "iofwd/LinkRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
      class IOFSLRPCLinkRequest :
          public RPCSimpleRequest<common::RPCLinkRequest, common::RPCLinkResponse>,
          public LinkRequest
      {
          public:
              IOFSLRPCLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCLinkRequest, common::RPCLinkResponse>(in, out),
                  LinkRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCLinkRequest();
              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                                  const zoidfs::zoidfs_cache_hint_t *
                                    from_parent_hint, 
                                  const zoidfs::zoidfs_cache_hint_t * 
                                    to_parent_hint);
          protected:
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
