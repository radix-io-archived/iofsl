#ifndef IOFWD_RPCFRONTEND_IOFSLRPCMKDIRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCMKDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/MkdirRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;

      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
      ENCODERSTRUCT (IOFSLRPCMkdirDec, ((EncoderString)(full_path)) 
                                        ((zoidfs_handle_t)(parent_handle))
                                        ((EncoderString)(component_name))              
                                        ((zoidfs_sattr_t)(sattr)))

      ENCODERSTRUCT (IOFSLRPCMkdirEnc, ((int)(returnCode))
                                       ((zoidfs_cache_hint_t)(parent_hint)))


      class IOFSLRPCMkdirRequest :
          public IOFSLRPCRequest,
          public MkdirRequest
      {
          public:
              IOFSLRPCMkdirRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  MkdirRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCMkdirRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                             const zoidfs::zoidfs_cache_hint_t * parent_hint);
          
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCMkdirDec dec_struct;
              IOFSLRPCMkdirEnc enc_struct;
              zoidfs::zoidfs_cache_hint_t * parent_hint;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif