#ifndef IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "iofwd/SetAttrRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "encoder/Util.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_attr_t zoidfs_attr_t;
      typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
      ENCODERSTRUCT (IOFSLRPCSetAttrDec, ((zoidfs_handle_t)(handle))
                                         ((zoidfs_attr_t)(attr))
                                         ((zoidfs_sattr_t)(sattr)))
      ENCODERSTRUCT (IOFSLRPCSetAttrEnc, ((int)(returnCode))
                                          ((zoidfs_attr_t)(attr)))

      class IOFSLRPCSetAttrRequest :
          public RPCSimpleRequest<IOFSLRPCSetAttrDec,IOFSLRPCSetAttrEnc>,
          public SetAttrRequest
      {
          public:
              IOFSLRPCSetAttrRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<IOFSLRPCSetAttrDec,IOFSLRPCSetAttrEnc>(in, out),
                  SetAttrRequest(opid)
              {
              }
            
               ~IOFSLRPCSetAttrRequest();

               const ReqParam & decodeParam ();
               void reply (const CBType & cb,
                 const zoidfs::zoidfs_attr_t * attr) ;
          protected:
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
