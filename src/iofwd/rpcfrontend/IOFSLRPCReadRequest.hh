#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ReadRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCReadRequest :
          public IOFSLRPCRequest,
          public ReadRequest
      {
          public:
              IOFSLRPCReadRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  ReadRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCReadRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              virtual ReqParam & decodeParam () = 0;

              virtual void reply(const CBType & cb) = 0;

              // for normal mode
              virtual void sendBuffers(const iofwdevent::CBType & cb, RetrievedBuffer * rb) = 0;

              // for pipeline mode
              virtual void sendPipelineBufferCB(const iofwdevent::CBType cb, RetrievedBuffer * rb, size_t size) = 0;

              virtual void initRequestParams(ReqParam & p, void * bufferMem) = 0;

              virtual void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb) = 0;

              virtual void releaseBuffer(RetrievedBuffer * rb) = 0;
                      
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
