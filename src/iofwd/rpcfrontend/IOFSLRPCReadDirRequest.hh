#ifndef IOFWD_RPCFRONTEND_IOFSLREADDIRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLREADDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ReadDirRequest.hh"
#include "iofwd/rpcfrontend/RPCSimpleRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {
      typedef zoidfs::zoidfs_dirent_t zoidfs_dirent_t;
      typedef zoidfs::zoidfs_op_hint_t zoidfs_op_hint_t;
      typedef zoidfs::zoidfs_handle_t zoidfs_handle_t;
      typedef zoidfs::zoidfs_dirent_cookie_t zoidfs_dirent_cookie_t;
      typedef zoidfs::zoidfs_cache_hint_t zoidfs_cache_hint_t;
      typedef encoder::EncoderString<0, ZOIDFS_PATH_MAX> EncoderString;

      ENCODERSTRUCT (IOFSLRPCReadDirDec, ((zoidfs_handle_t)(handle)) 
                                        ((zoidfs_dirent_cookie_t)(cookie))
                                        ((uint32_t)(entry_count))              
                                        ((uint32_t)(flags)))

      ENCODERSTRUCT (IOFSLRPCReadDirEnc, ((int)(returnCode))
                                         ((uint32_t)(entry_count))
                                         ((zoidfs_dirent_t)(entries))
                                         ((zoidfs_cache_hint_t)(cache)))

      class IOFSLRPCReadDirRequest :
          public RPCSimpleRequest<IOFSLRPCReadDirDec, IOFSLRPCReadDirEnc>,
          public ReadDirRequest
      {
          public:
              IOFSLRPCReadDirRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<IOFSLRPCReadDirDec, IOFSLRPCReadDirEnc>(in, out),
                  ReadDirRequest(opid)
              {
              }
            
              ~IOFSLRPCReadDirRequest();

              /* request processing */
              const ReqParam & decodeParam();
              void reply (const CBType & cb, uint32_t entry_count,
                                  zoidfs::zoidfs_dirent_t * entries,
                                  zoidfs::zoidfs_cache_hint_t * cache);
          
          protected:
              /* data size helpers for this request */ 
              ReqParam param_;
              zoidfs::zoidfs_op_hint_t op_hint_;
      };

   }
}

#endif
