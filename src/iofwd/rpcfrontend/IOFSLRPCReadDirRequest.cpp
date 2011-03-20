#include "iofwd/rpcfrontend/IOFSLRPCReadDirRequest.hh"
#include "iofwdutil/tools.hh"
#include "encoder/EncoderString.hh"
#include "IOFSLRPCGenProcess.hh"
namespace iofwd
{
   namespace rpcfrontend
   {
      RPC_GENPROCESS (IOFSLRPCReadDirRequest,((&)(handle)) 
                                             ((&)(cookie))
                                             (()(entry_count))              
                                             ((&)(entries))
                                             (()(flags)),
                                             (()(returnCode))
                                             (()(entry_count))
                                             ((*)(entries))
                                             ((*)(cache)))
                                            

      const IOFSLRPCReadDirRequest::ReqParam & IOFSLRPCReadDirRequest::decodeParam() 
      { 
          decodeRPCInput(); 
          param_.entry_count = dec_struct.entry_count;
          param_.entries = &dec_struct.entries;
          param_.flags = dec_struct.flags;
          param_.cookie = dec_struct.cookie;
          param_.op_hint = &op_hint_; 
          return param_; 
      }

      void IOFSLRPCReadDirRequest::reply(const CBType & cb, uint32_t entry_count_,
                                       zoidfs::zoidfs_dirent_t * entries_,
                                       zoidfs::zoidfs_cache_hint_t * cache_)
      {
          /* verify the args are OK */
          ASSERT(getReturnCode() != zoidfs::ZFS_OK || entries_ || cache_);

          /* store ptr to the attr */
          entries = entries_;
          cache = cache_;
          entry_count = entry_count_;
      
          /* encode */
          encodeRPCOutput();

          /* invoke the callback */
          //cb();
      }

      IOFSLRPCReadDirRequest::~IOFSLRPCReadDirRequest()
      {
         zoidfs::hints::zoidfs_hint_free(&op_hint_);
      }

      size_t IOFSLRPCReadDirRequest::rpcEncodedInputDataSize()
      {
          return 0;
      }

      size_t IOFSLRPCReadDirRequest::rpcEncodedOutputDataSize()
      {
          return 0;
      }

   }
}
