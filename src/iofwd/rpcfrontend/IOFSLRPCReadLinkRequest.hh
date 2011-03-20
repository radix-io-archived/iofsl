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
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;

      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
      ENCODERSTRUCT (IOFSLRPCReadLinkDec, ((zoidfs_handle_t)(handle))
                                          ((size_t)(buffer_length)))

      ENCODERSTRUCT (IOFSLRPCReadLinkEnc, ((int)(returnCode))
                                          ((EncoderString)(buffer))
                                          ((size_t)(buffer_length)))

      class IOFSLRPCReadLinkRequest :
          public IOFSLRPCRequest,
          public ReadLinkRequest
      {
          public:
              IOFSLRPCReadLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  ReadLinkRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCReadLinkRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, const char * buffer_,
                                  size_t buffer_length_);
                      
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              EncoderString buffer;
              size_t buffer_length;
              IOFSLRPCReadLinkDec dec_struct;
              IOFSLRPCReadLinkEnc enc_struct;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
