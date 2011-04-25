#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREADREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREADREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwdutil/ThreadPool.hh"

#include "iofwd/ReadRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "iofwdutil/mm/BMIMemoryManager.hh"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

namespace iofwd
{
   namespace rpcfrontend
   {

      typedef zoidfs::zoidfs_dirent_t zoidfs_dirent_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_dirent_cookie_t zoidfs_dirent_cookie_t;
      typedef zoidfs::zoidfs_file_ofs_t zoidfs_file_ofs_t;

      ENCODERSTRUCT (IOFSLRPCReadDec, ((zoidfs_handle_t)(handle_)) 
                                        ((size_t)(mem_count_))
                                        ((void**)(mem_starts_))              
                                        ((size_t*)(mem_sizes_))
                                        ((size_t)(file_count_))
                                        ((zoidfs_file_ofs_t*)(file_starts_))
                                        ((zoidfs_file_ofs_t*)(file_sizes_))
                                        ((size_t)(pipeline_size_)))

      ENCODERSTRUCT (IOFSLRPCReadEnc, ((int)(returnCode))
                                      ((zoidfs_file_ofs_t*)(file_starts))
                                      ((zoidfs_file_ofs_t*)(file_sizes)))

      class IOFSLRPCReadRequest :
          public IOFSLRPCRequest,
          public ReadRequest
      {
          public:
              IOFSLRPCReadRequest(iofwdutil::ThreadPool * tp,
                      int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  ReadRequest(opid)
              {
                tp_ = tp;
              }
            
              virtual ~IOFSLRPCReadRequest();

              /* encode and decode helpers for RPC data */
              void decode(const CBType & cb);
              void processDecode(const CBType & cb);
              void encode();

              ReqParam & decodeParam ();

              void reply(const CBType & cb);

              // for normal mode
              void sendBuffers(const iofwdevent::CBType & cb, 
                               RetrievedBuffer * rb);

              // for pipeline mode
              void sendPipelineBufferCB(const iofwdevent::CBType cb, 
                                        RetrievedBuffer * rb, size_t size);
              void sendPipelineBufferCBBlock (const iofwdevent::CBType cb, 
                                              RetrievedBuffer * rb, 
                                              size_t size);
              void initRequestParams(ReqParam & p, void * bufferMem);

              void allocateBuffer(const iofwdevent::CBType cb, RetrievedBuffer * rb);
              void releaseBuffer(RetrievedBuffer * rb);
              size_t writeBuffer(void * buff, size_t size, bool flush);
              void sendBuffersBlock(const iofwdevent::CBType & cb, RetrievedBuffer * rb);
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

             ReqParam param_;

             zoidfs::zoidfs_op_hint_t op_hint_;

             // @TODO: This should not be specified here. however its required
             //        to use the memory manager (BMIMemoryManager)
             IOFSLRPCReadEnc enc_struct;
             IOFSLRPCReadDec dec_struct;

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
             size_t total_write;
             iofwdutil::ThreadPool * tp_;
      };

   }
}

#endif
