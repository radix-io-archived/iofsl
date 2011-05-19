#ifndef IOFWD_RPCFRONTEND_IOFSLRPCGETATTRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCGETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "iofwd/GetAttrRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "encoder/Util.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_attr_t zoidfs_attr_t;
      ENCODERSTRUCT (IOFSLRPCGetAttrDec, ((zoidfs_handle_t)(handle))
                                         ((zoidfs_attr_t)(attr)))
      ENCODERSTRUCT (IOFSLRPCGetAttrEnc, ((int)(returnCode))
                                          ((zoidfs_attr_t)(attr_enc)))

      class IOFSLRPCGetAttrRequest :
          public RPCSimpleRequest<IOFSLRPCGetAttrDec, IOFSLRPCGetAttrEnc>,
          public GetAttrRequest
      {
          public:
              IOFSLRPCGetAttrRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<IOFSLRPCGetAttrDec, IOFSLRPCGetAttrEnc>(in,out),
                  GetAttrRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCGetAttrRequest();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb,
                      const zoidfs::zoidfs_attr_t * attr);
          
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
