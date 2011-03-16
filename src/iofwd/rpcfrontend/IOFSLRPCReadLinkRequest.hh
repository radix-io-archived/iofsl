#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ReadLinkRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCReadLinkRequest :
          public IOFSLRPCRequest,
          public ReadLinkRequest
      {
          public:
              IOFSLRPCReadLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  ReadLinkRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCReadLinkRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, const char * buffer,
                                  size_t buffer_length) = 0;
                      
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
