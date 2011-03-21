#ifndef IOFWD_RPCFRONTEND_IOFSLRPCSYMLINKREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCSYMLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/SymLinkRequest.hh"
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
      ENCODERSTRUCT (IOFSLRPCSymLinkDec, ((EncoderString)(from_full_path)) 
                                        ((zoidfs_handle_t)(from_parent_handle))
                                        ((EncoderString)(from_component_name))              
                                        ((EncoderString)(to_full_path))
                                        ((zoidfs_handle_t)(to_parent_handle))
                                        ((EncoderString)(to_component_name))
                                        ((zoidfs_sattr_t)(sattr)))

      ENCODERSTRUCT (IOFSLRPCSymLinkEnc, ((int)(returnCode))
                                        ((zoidfs_cache_hint_t)(from_parent_hint))
                                        ((zoidfs_cache_hint_t)(to_parent_hint)))
      class IOFSLRPCSymLinkRequest :
          public IOFSLRPCRequest,
          public SymLinkRequest
      {
          public:
              IOFSLRPCSymLinkRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  SymLinkRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCSymLinkRequest();

              /* encode and decode helpers for RPC data */
              void decode();
              void encode();

              /**
              * Retrieve the request input parameters
              */
              const ReqParam & decodeParam ()  = 0;

              /**
              * Reply with the handle or 0 if an error occurred and the handle does not
              * need to be transmitted
              */
              void reply (const CBType & cb,
                  const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                  const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              IOFSLRPCSymLinkDec dec_struct;
              IOFSLRPCSymLinkEnc enc_struct;
              zoidfs::zoidfs_cache_hint_t * from_parent_hint;
              zoidfs::zoidfs_cache_hint_t * to_parent_hint;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
