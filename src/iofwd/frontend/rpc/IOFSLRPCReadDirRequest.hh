#ifndef IOFWD_RPCFRONTEND_IOFSLREADDIRREQUEST_HH
#define IOFWD_RPCFRONTEND_IOFSLREADDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "zoidfs/zoidfs.h"

#include "iofwd/ReadDirRequest.hh"
#include "iofwd/frontend/rpc/RPCSimpleRequest.hh"

#include "common/rpc/CommonRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

      class IOFSLRPCReadDirRequest :
          public RPCSimpleRequest<common::RPCReadDirRequest, common::RPCReadDirResponse>,
          public ReadDirRequest
      {
          public:
              IOFSLRPCReadDirRequest(int opid,
                      iofwdevent::ZeroCopyInputStream * in,
                      iofwdevent::ZeroCopyOutputStream * out) :
                  RPCSimpleRequest<common::RPCReadDirRequest, common::RPCReadDirResponse>(in, out),
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
