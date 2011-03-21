#ifndef IOFWD_RPCFRONTEND_IOFSLRPCREMOVEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCREMOVEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/RemoveRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "encoder/EncoderWrappers.hh"
using namespace encoder;

namespace iofwd
{
   namespace rpcfrontend
   {

      typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;
      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;

      ENCODERSTRUCT (IOFSLRPCRemoveDec, ((EncoderString)(full_path))
                                        ((EncoderString)(component_name))
                                        ((zoidfs_handle_t)(parent_handle)))

      ENCODERSTRUCT (IOFSLRPCRemoveEnc, ((int)(returnCode))
                                        ((zoidfs_cache_hint_t)(parent_hint)))

      class IOFSLRPCRemoveRequest :
          public IOFSLRPCRequest,
          public RemoveRequest
      {
          public:
              IOFSLRPCRemoveRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  RemoveRequest(opid)
              {
              }

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                              const zoidfs::zoidfs_cache_hint_t * parent_hint_);
              ~IOFSLRPCRemoveRequest();
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCRemoveDec dec_struct;
              IOFSLRPCRemoveEnc enc_struct;
              zoidfs::zoidfs_cache_hint_t * parent_hint;

              zoidfs::zoidfs_op_hint_t op_hint_;
      };
   }
}
#endif
