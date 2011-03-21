#ifndef IOFWD_RPCFRONTEND_IOFSLRPCWRITEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCWRITEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/WriteRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "iofwdutil/mm/BMIMemoryManager.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      typedef zoidfs::zoidfs_dirent_t zoidfs_dirent_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_dirent_cookie_t zoidfs_dirent_cookie_t;
      typedef zoidfs::zoidfs_file_ofs_t zoidfs_file_ofs_t;

      ENCODERSTRUCT (IOFSLRPCWriteDec, ((zoidfs_handle_t)(handle)) 
                                        ((size_t)(mem_count))
                                        ((void**)(mem_starts))              
                                        ((size_t*)(mem_sizes))
                                        ((size_t)(file_count))
                                        ((zoidfs_file_ofs_t)(file_starts))
                                        ((zoidfs_file_ofs_t)(file_sizes))
                                        ((size_t)(pipeline_size)))


      ENCODERSTRUCT (IOFSLRPCWriteEnc, ((int)(returnCode))
                                       ((zoidfs_file_ofs_t)(file_starts))
                                       ((zoidfs_file_ofs_t)(file_sizes)))


      class IOFSLRPCWriteRequest :
          public IOFSLRPCRequest,
          public WriteRequest
      {
          public:
              IOFSLRPCWriteRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  WriteRequest(opid)
              {
              }
            
              ~IOFSLRPCWriteRequest();

              /* encode and decode helpers for RPC data */
              void decode();
              void encode();

              ReqParam & decodeParam () = 0;

              void reply(const CBType & cb) = 0;

              // for normal mode
              void recvBuffers(const CBType & cb, RetrievedBuffer * rb) = 0;

              // for pipeline mode
              void recvPipelineBufferCB(iofwdevent::CBType cb, 
                                                RetrievedBuffer * rb, 
                                                size_t size) = 0;

              void initRequestParams(ReqParam & p, void * bufferMem) = 0;

              void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb) = 0;

              void releaseBuffer(RetrievedBuffer * rb) = 0;

              size_t readBuffer (void * buff, size_t size, bool forceSize);


          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;

              IOFSLRPCWriteEnc enc_struct;
              IOFSLRPCWriteDec dec_struct;
              // @TODO: This should not be specified here. however its required
              //        to use the memory manager (BMIMemoryManager)
              iofwdutil::bmi::BMIAddr * addr_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
