#ifndef IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/CreateRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCCreateRequest :
          public IOFSLRPCRequest,
          public CreateRequest
      {
          public:
              IOFSLRPCCreateRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  CreateRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCCreateRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb, 
                                 const zoidfs::zoidfs_attr_t * attr, 
                                 int created);
          
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
