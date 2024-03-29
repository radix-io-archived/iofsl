#ifndef IOFWD_RPCFRONTEND_IOFSLRPCWRITEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCWRITEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwdutil/ThreadPool.hh"

#include "iofwd/WriteRequest.hh"
#include "iofwd/frontend/rpc/IOFSLRPCRequest.hh"
#include "iofwdutil/mm/BMIMemoryManager.hh"

#include "zoidfs/util/ZoidfsFileOfsStruct.hh"

#include "common/rpc/CommonRequest.hh"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace iofwd
{
   namespace rpcfrontend
   {
      class IOFSLRPCWriteRequest :
          public IOFSLRPCRequest,
          public WriteRequest
      {
          public:
              IOFSLRPCWriteRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  WriteRequest(opid),
                  total_read(0)
                  
              {
              }
            
              ~IOFSLRPCWriteRequest();

              /* encode and decode helpers for RPC data */
              void decode(const CBType & cb);
              void processDecode(const CBType & cb);

              /* Encode Functions */
              void reply(const CBType & cb);
              void encode(const CBType & cb);
              void encodeWrite(iofwdevent::CBException e, const CBType & cb);
              void encodeFlush(iofwdevent::CBException e, const CBType & cb);

              /* Included until this can be removed from base class RPCRequest */
              void encode();
              
              ReqParam & decodeParam ();

              // for normal mode
              void recvBuffers(const CBType & cb, RetrievedBuffer * rb);

              // for pipeline mode
              void recvPipelineBufferCB(iofwdevent::CBType cb, 
                                        iofwd::RetrievedBuffer* rb, 
                                        size_t size);

              void recvRead (iofwdevent::CBType cb, 
                             iofwd::RetrievedBuffer* rb, size_t size,
                             size_t * outSize, size_t * runSize);

              void recvCheck (iofwdevent::CBException e,
                              iofwdevent::CBType cb,
                              iofwd::RetrievedBuffer* rb, 
                              size_t size,
                              size_t * outSize,
                              size_t * runSize);

              void readBuffer (CBType cb, void * buff, size_t sizdec_, 
                               size_t * outSize);

              void bufferRecv (iofwdevent::CBException e, CBType cb,
                               void * buff, size_t sizdec_,
                               size_t * outSize, void ** tmpBuffer);

              void initRequestParams(ReqParam & p, void * bufferMem);

              void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb);

              void releaseBuffer(RetrievedBuffer * rb);

              void preformDecode(const CBType & cb);
          protected:
              /* data size helpers for this request */ 
               size_t rpcEncodedInputDataSize(); 
               size_t rpcEncodedOutputDataSize();

              ReqParam param_;

              common::RPCWriteResponse enc_struct;
              common::RPCWriteRequest dec_struct;
              // @TODO: This should not be specified here. however its required
              //        to use the memory manager (BMIMemoryManager)
              iofwdutil::bmi::BMIAddr *  addr_;
              zoidfs::zoidfs_op_hint_t op_hint_;

             /* pointers and sizes of mem stream data */
             const char * read_ptr_;
             size_t read_size_;
             char * write_ptr_;
             size_t write_size_;
             size_t insize_;
             size_t outsize_;

             /* RPC encoder / decoder */
             rpc::RPCDecoder dec_;
             rpc::RPCEncoder enc_;
             size_t total_read;
      };

   }
}

#endif
