#ifndef IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/CreateRequest.hh"
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
      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;
      ENCODERSTRUCT (IOFSLRPCCreateDec, ((zoidfs_handle_t)(handle)) 
                                        ((EncoderString)(full_path))
                                        ((EncoderString)(component_name))              
                                        ((zoidfs_sattr_t)(attr)))

      ENCODERSTRUCT (IOFSLRPCCreateEnc, ((int)(returnCode))
                                        ((zoidfs_handle_t)(handle))
                                        ((int)(created)))
      
      class IOFSLRPCCreateRequest :
          public IOFSLRPCRequest,
          public CreateRequest
      {
          public:
              IOFSLRPCCreateRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  IOFSLRPCRequest(in, out),
                  CreateRequest(opid),
                  created(0),
                  handle(NULL)
              {
              }
            
              virtual ~IOFSLRPCCreateRequest();

              /* encode and decode helpers for RPC data */
              virtual void decode();
              virtual void encode();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb, 
                                 const zoidfs::zoidfs_handle_t * handle_, 
                                 int created_);
          
          protected:
              /* data size helpers for this request */ 
              virtual size_t rpcEncodedInputDataSize(); 
              virtual size_t rpcEncodedOutputDataSize();

              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;

              IOFSLRPCCreateDec dec_struct;
              IOFSLRPCCreateEnc enc_struct;
              encoder::EncoderString<0, ZOIDFS_PATH_MAX> full_path;
              encoder::EncoderString<0, ZOIDFS_PATH_MAX> component_name;
              zoidfs_handle_t * handle;
              int created;
      };

   }
}

#endif
