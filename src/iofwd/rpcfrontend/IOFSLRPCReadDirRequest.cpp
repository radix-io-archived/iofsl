#include "iofwd/rpcfrontend/IOFSLRPCReadDirRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {

      const IOFSLRPCReadDirRequest::ReqParam & IOFSLRPCReadDirRequest::decodeParam() 
      { 
          param_.handle = &inStruct.handle;
          param_.entry_count = inStruct.entry_count;
          param_.entries = new zoidfs::zoidfs_dirent_t[inStruct.entry_count];
          param_.flags = inStruct.flags;
          param_.cookie = inStruct.cookie;
          param_.op_hint = NULL;
          return param_; 
      }

      void IOFSLRPCReadDirRequest::reply(const CBType & cb, uint32_t entry_count_,
                                       zoidfs::zoidfs_dirent_t * entries_,
                                       zoidfs::zoidfs_cache_hint_t * cache_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() == zoidfs::ZFS_OK);

          /* store ptr to the attr */
          outStruct.entries.list = entries_;
          outStruct.entries.count = entry_count_;
          outStruct.cache = *cache_;
          outStruct.returnCode = getReturnCode();

          zoidfs::hints::zoidfs_hint_create(&op_hint_);  
          /* @TODO: Remove this later */
          param_.op_hint = &op_hint_;

          encode(cb);
      }

      IOFSLRPCReadDirRequest::~IOFSLRPCReadDirRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

   }
}
