#ifndef IOFWD_RPCFRONTEND_IOFSLRPCSYMLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCSYMLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "iofwd/SymLinkRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCSymLinkRequest :
          public RPCSimpleRequest<common::RPCSymlinkRequest,common::RPCSymlinkResponse>,
          public SymLinkRequest
      {
          public:
              IOFSLRPCSymLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCSymlinkRequest,common::RPCSymlinkResponse>(in, out),
                  SymLinkRequest(opid)
              {
              }
            
               ~IOFSLRPCSymLinkRequest();

              /**
              * Retrieve the request input parameters
              */
              const ReqParam & decodeParam ();

              /**
              * Reply with the handle or 0 if an error occurred and the handle does not
              * need to be transmitted
              */
              void reply (const CBType & cb,
                  const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                  const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
