#ifndef IOFWD_RPCFRONTEND_IOFSLRPCLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/LinkRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;

      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
      ENCODERSTRUCT (IOFSLRPCLinkDec, ((EncoderString)(from_full_path)) 
                                        ((zoidfs_handle_t)(from_parent_handle))
                                        ((EncoderString)(from_component_name))              
                                        ((EncoderString)(to_full_path))
                                        ((zoidfs_handle_t)(to_parent_handle))
                                        ((EncoderString)(to_component_name)))

      ENCODERSTRUCT (IOFSLRPCLinkEnc, ((int)(returnCode))
                                        ((zoidfs_cache_hint_t)(from_parent_hint))
                                        ((zoidfs_cache_hint_t)(to_parent_hint)))
      class IOFSLRPCLinkRequest :
          public IOFSLRPCRequest,
          public LinkRequest
      {
          public:
              IOFSLRPCLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  LinkRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCLinkRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply (const CBType & cb, 
                                  const zoidfs::zoidfs_cache_hint_t *
                                    from_parent_hint, 
                                  const zoidfs::zoidfs_cache_hint_t * 
                                    to_parent_hint);
          
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCLinkDec dec_struct;
              IOFSLRPCLinkEnc enc_struct;
              zoidfs::zoidfs_cache_hint_t * from_parent_hint;
              zoidfs::zoidfs_cache_hint_t * to_parent_hint;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
