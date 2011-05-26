#ifndef IOFWD_RPCFRONTEND_IOFSLRPCMKDIRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCMKDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/MkdirRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"

#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCMkdirRequest :
          public RPCSimpleRequest<common::RPCMkdirRequest, common::RPCMkdirResponse>,
          public MkdirRequest
      {
          public:
              IOFSLRPCMkdirRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCMkdirRequest, common::RPCMkdirResponse>(in, out),
                  MkdirRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCMkdirRequest();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                             const zoidfs::zoidfs_cache_hint_t * parent_hint);
          
          protected:
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
