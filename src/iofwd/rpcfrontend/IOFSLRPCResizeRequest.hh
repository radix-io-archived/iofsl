#ifndef IOFWD_RPCFRONTEND_IOFSLRPCRESIZEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCRESIZEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ResizeRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCResizeRequest :
          public IOFSLRPCRequest,
          public ResizeRequest
      {
          public:
              IOFSLRPCResizeRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  ResizeRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCResizeRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              virtual const ReqParam & decodeParam () = 0;

              virtual void reply (const CBType & cb) = 0;
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
