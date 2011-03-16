#ifndef IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/SetAttrRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCSetAttrRequest :
          public IOFSLRPCRequest,
          public SetAttrRequest
      {
          public:
              IOFSLRPCSetAttrRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  SetAttrRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCSetAttrRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              virtual const ReqParam & decodeParam () = 0;
              virtual void reply (const CBType & cb,
                 const zoidfs::zoidfs_attr_t * attr) = 0;
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
