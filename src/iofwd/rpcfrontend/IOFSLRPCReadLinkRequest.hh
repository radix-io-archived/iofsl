#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREADLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ReadLinkRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
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
          public RPCSimpleRequest<IOFSLRPCReadLinkDec, IOFSLRPCReadLinkEnc>,
          public ReadLinkRequest
      {
          public:
              IOFSLRPCReadLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<IOFSLRPCReadLinkDec, IOFSLRPCReadLinkEnc>(in, out),
                  ReadLinkRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCReadLinkRequest();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, const char * buffer_,
                                  size_t buffer_length_);
                      
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
