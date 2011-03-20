#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREMOVEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREMOVEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/RemoveRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCRemoveRequest :
          public IOFSLRPCRequest,
          public RemoveRequest
      {
          public:
              IOFSLRPCRemoveRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  RemoveRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCRemoveRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /**
              * Retrieve the request input parameters 
              */
              virtual const ReqParam & decodeParam ()  = 0; 

              /**
              * Reply with the handle or 0 if an error occurred and the handle does not
              * need to be transmitted
              */
              virtual void reply (const CBType & cb,
                 const zoidfs::zoidfs_cache_hint_t * parent_hint) = 0;
                              
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