#ifndef IOFWD_RPCFRONTEND_IOFSLRPCWRITEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCWRITEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/WriteRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCGetAttrRequest :
          public IOFSLRPCRequest,
          public WriteRequest
      {
          public:
              IOFSLRPCWriteRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  WriteRequest(opid),
                  attr_enc_(NULL)
              {
              }
            
              virtual ~IOFSLRPCWriteRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam() = 0;
              virtual void reply(const CBType & cb,
                                 const zoidfs::zoidfs_attr_t * attr) = 0;
              virtual ReqParam & decodeParam () = 0;
  
              virtual void reply(const CBType & cb) = 0;
              
//               for normal mode
//              virtual void recvBuffers(const CBType & cb, RetrievedBuffer * rb);

//               for pipeline mode
//              virtual void recvPipelineBufferCB(iofwdevent::CBType cb, 
//                                                RetrievedBuffer * rb, 
//                                                size_t size);

//              virtual void initRequestParams(ReqParam & p, void * bufferMem);

//              virtual void allocateBuffer(iofwdevent::CBType cb, 
//                                          RetrievedBuffer * rb);

//              virtual void releaseBuffer(RetrievedBuffer * rb);

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
