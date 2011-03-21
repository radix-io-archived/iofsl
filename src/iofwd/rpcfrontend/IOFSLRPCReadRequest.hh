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
//      zoidfs::zoidfs_handle_t * handle;
//      size_t mem_count;
//      void ** mem_starts;
//      size_t * mem_sizes;
//      size_t file_count;
//      zoidfs::zoidfs_file_ofs_t * file_starts;
//      zoidfs::zoidfs_file_ofs_t * file_sizes;
//      size_t pipeline_size;
//      zoidfs::zoidfs_op_hint_t * op_hint;

//      typedef zoidfs::zoidfs_dirent_t zoidfs_dirent_t;
//      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
//      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
//      typedef zoidfs::zoidfs_dirent_cookie_t zoidfs_dirent_cookie_t;
//      typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;
//      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;

//      ENCODERSTRUCT (IOFSLRPCReadDirDec, ((zoidfs_handle_t)(handle)) 
//                                        ((zoidfs_dirent_cookie_t)(cookie))
//                                        ((uint32_t)(entry_count))              
//                                        ((zoidfs_dirent_t)(entries))
//                                        ((uint32_t)(flags)))

//      ENCODERSTRUCT (IOFSLRPCReadDirEnc, ((int)(returnCode))
//                                         ((uint32_t)(entry_count))
//                                         ((zoidfs_dirent_t)(entries))
//                                         ((zoidfs_cache_hint_t)(cache)))

      /* decoded hint values */
      bool op_hint_pipeline_enabled;

      size_t max_buffer_size;
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
