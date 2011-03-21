#ifndef IOFWD_RPCFRONTEND_IOFSLRPCRENAMEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCRENAMEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/RenameRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "encoder/EncoderWrappers.hh"
using namespace encoder;

namespace iofwd
{
   namespace rpcfrontend
   {

      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;

      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
      ENCODERSTRUCT (IOFSLRPCRenameDec, ((EncoderString)(from_full_path)) 
                                        ((zoidfs_handle_t)(from_parent_handle))
                                        ((EncoderString)(from_component_name))              
                                        ((EncoderString)(to_full_path))
                                        ((zoidfs_handle_t)(to_parent_handle))
                                        ((EncoderString)(to_component_name)))

      ENCODERSTRUCT (IOFSLRPCRenameEnc, ((int)(returnCode))
                                        ((zoidfs_cache_hint_t)(from_parent_hint))
                                        ((zoidfs_cache_hint_t)(to_parent_hint)))

      class IOFSLRPCRenameRequest :
          public IOFSLRPCRequest,
          public RenameRequest
      {
          public:
              IOFSLRPCRenameRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  RenameRequest(opid)
              {
              }

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                                  const zoidfs::zoidfs_cache_hint_t *
                                    from_parent_hint_, 
                                  const zoidfs::zoidfs_cache_hint_t * 
                                    to_parent_hint_);
          
              ~IOFSLRPCRenameRequest();
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCRenameDec dec_struct;
              IOFSLRPCRenameEnc enc_struct;
              zoidfs::zoidfs_cache_hint_t * from_parent_hint;
              zoidfs::zoidfs_cache_hint_t * to_parent_hint;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };
   }
}
#endif
