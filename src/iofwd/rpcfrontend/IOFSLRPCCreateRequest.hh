#ifndef IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLRPCCREATEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"
#include "encoder/EncoderStruct.hh"
#include "encoder/EncoderString.hh"
#include "iofwd/CreateRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"
#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"
#include "encoder/EncoderWrappers.hh"
#include "zoidfs/util/ZoidFSFileSpec.hh"
using namespace encoder;

namespace iofwd
{
   namespace rpcfrontend
   {
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_sattr_t zoidfs_sattr_t;
      typedef zoidfs::ZoidFSFileSpec ZoidFSFileSpec;
      ENCODERSTRUCT (RPCCreateIn, ((ZoidFSFileSpec)(info))            
                                  ((zoidfs_sattr_t)(attr)))

      ENCODERSTRUCT (RPCCreateOut, ((int)(returnCode))
                                   ((zoidfs_handle_t)(handle))
                                   ((int)(created)))
      
      class IOFSLRPCCreateRequest :
          public RPCSimpleRequest<RPCCreateIn, RPCCreateOut>,
          public CreateRequest
      {
          public:
              IOFSLRPCCreateRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<RPCCreateIn, RPCCreateOut>(in, out),
                  CreateRequest(opid)
              {
              }
            
              virtual ~IOFSLRPCCreateRequest();

              /* request processing */
              virtual const ReqParam & decodeParam();
              virtual void reply(const CBType & cb, 
                                 const zoidfs::zoidfs_handle_t * handle_, 
                                 int created_);
          
          protected:
              ReqParam param_;
      };

   }
}

#endif
