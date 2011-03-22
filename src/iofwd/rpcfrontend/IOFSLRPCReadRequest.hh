#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREADREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREADREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ReadRequest.hh"
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

      ENCODERSTRUCT (IOFSLRPCReadDec, ((zoidfs_handle_t)(handle)) 
                                        ((size_t)(mem_count))
                                        ((void**)(mem_starts))              
                                        ((size_t*)(mem_sizes))
                                        ((size_t)(file_count))
                                        ((zoidfs_file_ofs_t)(file_starts))
                                        ((zoidfs_file_ofs_t)(file_sizes))
                                        ((size_t)(pipeline_size)))

      ENCODERSTRUCT (IOFSLRPCReadEnc, ((int)(returnCode))
                                      ((zoidfs_file_ofs_t)(file_starts))
                                      ((zoidfs_file_ofs_t)(file_sizes)))

      class IOFSLRPCReadRequest :
          public IOFSLRPCRequest,
          public ReadRequest
      {
          public:
              IOFSLRPCReadRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  ReadRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCReadRequest();

              /* encode and decode helpers for RPC data */
              void decode();
              void encode();

              ReqParam & decodeParam ();

              void reply(const CBType & cb);

              // for normal mode
              void sendBuffers(const iofwdevent::CBType & cb, 
                               RetrievedBuffer * rb);

              // for pipeline mode
              void sendPipelineBufferCB(const iofwdevent::CBType cb, 
                                        RetrievedBuffer * rb, size_t size);

              void initRequestParams(ReqParam & p, void * bufferMem);

              void allocateBuffer(iofwdevent::CBType cb, RetrievedBuffer * rb);
              void releaseBuffer(RetrievedBuffer * rb);
              void writeBuffer(void * buff, size_t size, bool flush);
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;

              zoidfs::zoidfs_op_hint_t op_hint_;

              // @TODO: This should not be specified here. however its required
              //        to use the memory manager (BMIMemoryManager)
              iofwdutil::bmi::BMIAddr * addr_;
              IOFSLRPCReadEnc enc_struct;
              IOFSLRPCReadDec dec_struct;
      };

   }
}

#endif
