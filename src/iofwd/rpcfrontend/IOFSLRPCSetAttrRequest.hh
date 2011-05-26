#ifndef IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/SetAttrRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCSetAttrRequest :
          public RPCSimpleRequest<common::RPCSetAttrRequest,common::RPCSetAttrResponse>,
          public SetAttrRequest
      {
          public:
              IOFSLRPCSetAttrRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCSetAttrRequest,common::RPCSetAttrResponse>(in, out),
                  SetAttrRequest(opid)
              {
              }
            
               ~IOFSLRPCSetAttrRequest();

               const ReqParam & decodeParam ();
               void reply (const CBType & cb,
                 const zoidfs::zoidfs_attr_t * attr) ;
          protected:
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
