#ifndef IOFWD_RPCFRONTEND_IOFSLRPCRESIZEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCRESIZEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/ResizeRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
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
          public RPCSimpleRequest<IOFSLRPCResizeDec, IOFSLRPCResizeEnc>,
          public ResizeRequest
      {
          public:
              IOFSLRPCResizeRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<IOFSLRPCResizeDec, IOFSLRPCResizeEnc>(in, out),
                  ResizeRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCResizeRequest();
              const ReqParam & decodeParam ();
              void reply (const CBType & cb);
          protected:
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
