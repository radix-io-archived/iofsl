#ifndef IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCSETATTRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "iofwd/SetAttrRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
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
          public IOFSLRPCRequest,
          public SetAttrRequest
      {
          public:
              IOFSLRPCSetAttrRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  SetAttrRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCSetAttrRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              virtual const ReqParam & decodeParam () = 0;
              virtual void reply (const CBType & cb,
                 const zoidfs::zoidfs_attr_t * attr) = 0;
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCSetAttrDec dec_struct;
              IOFSLRPCSetAttrEnc enc_struct;
              zoidfs::zoidfs_op_hint_t op_hint_;
              zoidfs::zoidfs_attr_t * attr;
      };

   }
}

#endif
