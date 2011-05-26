#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREADREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREADREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwdutil/ThreadPool.hh"

#include "iofwd/ReadRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "iofwdutil/mm/BMIMemoryManager.hh"

#include "zoidfs/util/ZoidfsFileOfsStruct.hh"

#include "common/rpc/CommonRequest.hh"

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
                  header_sent(false)
              {
              }
            
              virtual ~IOFSLRPCReadRequest();

              /* encode and decode helpers for RPC data */
              void decode(const CBType & cb);
              void processDecode(const CBType & cb);
              void encode(CBType cb);
              void writeEncode (iofwdevent::CBException e, CBType cb);
              void encodeFlush (iofwdevent::CBException e, CBType cb);


              void encode() {};
              

              ReqParam & decodeParam ();

              void reply(const CBType & cb);

              // for normal mode
              void sendBuffers(const iofwdevent::CBType & cb, 
                               RetrievedBuffer * rb);

              // for pipeline mode
              void sendPipelineBufferCB(const iofwdevent::CBType cb, 
                                        RetrievedBuffer * rb, size_t size);

              void sendNext ( iofwdevent::CBException e,
                              const iofwdevent::CBType cb, 
                              RetrievedBuffer * rb, 
                              size_t size,
                              size_t * outSize,
                              size_t * readSize,
                              size_t * readLoc);

              void sendCheck ( iofwdevent::CBException e,
                               const iofwdevent::CBType cb, 
                               RetrievedBuffer * rb, 
                               size_t size,
                               size_t * outSize,
                               size_t * readSize,
                               size_t * readLoc);

              void writeGetBuffer(CBType cb, void * buff, 
                                  size_t size, size_t * readSize);

              void writeGotBuffer(iofwdevent::CBException e, 
                                  CBType cb, void * buff, 
                                  size_t size, size_t * readSize,
                                  size_t * outsize, char ** writePtr);

              void writeFlush (iofwdevent::CBException e, CBType cb, size_t * outsize, 
                               char ** writePtr);


              void initRequestParams(ReqParam & p, void * bufferMem);

              void allocateBuffer(const iofwdevent::CBType cb, RetrievedBuffer * rb);
              void releaseBuffer(RetrievedBuffer * rb);
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

             ReqParam param_;

             zoidfs::zoidfs_op_hint_t op_hint_;

             // @TODO: This should not be specified here. however its required
             //        to use the memory manager (BMIMemoryManager)
             common::RPCReadResponse enc_struct;
             common::RPCReadRequest dec_struct;

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
             bool header_sent;
	
	     size_t total_read_size;
      };

   }
}

#endif
