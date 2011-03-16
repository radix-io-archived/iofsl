#ifndef IOFWD_RPCFRONTEND_IOFSLRPCLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/LinkRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCLinkRequest :
          public IOFSLRPCRequest,
          public LinkRequest
      {
          public:
              IOFSLRPCCreateRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  LinkRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCCreateRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                                  const zoidfs::zoidfs_cache_hint_t *
                                    from_parent_hint, 
                                  const zoidfs::zoidfs_cache_hint_t * 
                                    to_parent_hint) = 0;
          
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              zoidfs::zoidfs_handle_t handle_;
              zoidfs::zoidfs_attr_t attr_;
              zoidfs::zoidfs_op_hint_t op_hint_;
              zoidfs::zoidfs_attr_t * attr_enc_;
      };

   }
}

#endif
