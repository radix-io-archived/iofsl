#ifndef IOFWD_RPCFRONTEND_IOFSLRPCRESIZEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCRESIZEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/ResizeRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "encoder/EncoderWrappers.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;

      ENCODERSTRUCT (IOFSLRPCResizeDec, ((zoidfs_handle_t)(handle))
                                        ((size_t)(size)))

      ENCODERSTRUCT (IOFSLRPCResizeEnc, ((int)(returnCode)))

      class IOFSLRPCResizeRequest :
          public IOFSLRPCRequest,
          public ResizeRequest
      {
          public:
              IOFSLRPCResizeRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  ResizeRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCResizeRequest();

              /* encode and decode helpers for RPC data */
              void decode();
              void encode();

              const ReqParam & decodeParam ();

              void reply (const CBType & cb);
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCResizeDec dec_struct;
              IOFSLRPCResizeEnc enc_struct;

              zoidfs::zoidfs_op_hint_t op_hint_;

      };

   }
}

#endif
