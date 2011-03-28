#include "iofwdclient/SyncClient.hh"
#include "iofwdclient/ASClient.hh"

#include "iofwdclient/ScopedRequest.hh"

#include "iofwdutil/IOFWDLog.hh"

using namespace zoidfs;

namespace iofwdclient
{
   //========================================================================

   SyncClient::SyncClient (iofwdutil::IOFWDLogSource & log, ASClient & as)
      : log_ (log),
        asclient_ (as)
   {
      ZLOG_DEBUG (log_, "Initializing SyncClient");
   }

   SyncClient::~SyncClient ()
   {
      ZLOG_DEBUG (log_, "Shutting down SyncClient");
   }
//   // @TODO: This needs to be changed, possibly use the CommStream class?
//   void SyncClient::setRPCMode (boost::shared_ptr<iofwd::RPCClient> rpcclient,
//                                net::AddressPtr addr)
//   {
//      asclient_.setRPCMode (rpcclient, addr);
//   } 
   // @TODO: repeat for other functions

   int SyncClient::getattr (const zoidfs_handle_t * handle,
         zoidfs_attr_t * attr, zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.igetattr (&req, handle, attr, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret);
   }
   int SyncClient::setattr(const zoidfs::zoidfs_handle_t *handle,
                            const zoidfs::zoidfs_sattr_t *sattr,
                            zoidfs::zoidfs_attr_t *attr, 
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.isetattr(&req, handle, sattr, attr, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

   int SyncClient::lookup( const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, 
                           const char *full_path,
                           zoidfs::zoidfs_handle_t *handle,  
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.ilookup(&req, parent_handle, component_name, full_path, 
                              handle, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

   int SyncClient::readlink(const zoidfs::zoidfs_handle_t *handle, 
                             char *buffer,
                             size_t buffer_length,
                             zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.ireadlink(&req, handle, buffer, buffer_length, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

                      
   int SyncClient::commit(const zoidfs::zoidfs_handle_t *handle,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.icommit (&req, handle, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }
   
   int SyncClient::create(const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, 
                           const char *full_path,
                           const zoidfs::zoidfs_sattr_t *sattr, 
                           zoidfs::zoidfs_handle_t *handle,
                           int *created,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.icreate (&req, parent_handle, component_name, full_path, 
                               sattr, handle, created, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }  
   
              
   int SyncClient::remove(const zoidfs::zoidfs_handle_t *parent_handle,
                           const char *component_name, const char *full_path,
                           zoidfs::zoidfs_cache_hint_t *parent_hint,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.iremove(&req, parent_handle, component_name, full_path, 
                              parent_hint, op_hint);   
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

   int SyncClient::rename(const zoidfs::zoidfs_handle_t *from_parent_handle,
                           const char *from_component_name,
                           const char *from_full_path,
                           const zoidfs::zoidfs_handle_t *to_parent_handle,
                           const char *to_component_name,
                           const char *to_full_path,
                           zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                           zoidfs::zoidfs_cache_hint_t *to_parent_hint,     
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.irename(&req, from_parent_handle, from_component_name, 
                              from_full_path, to_parent_handle,
                              to_component_name, to_full_path,
                              from_parent_hint, to_parent_hint, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

   int SyncClient::link(const zoidfs::zoidfs_handle_t *from_parent_handle,
                         const char *from_component_name,
                         const char *from_full_path,
                         const zoidfs::zoidfs_handle_t *to_parent_handle,
                         const char *to_component_name,
                         const char *to_full_path,
                         zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                         zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                         zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.ilink(&req, from_parent_handle, from_component_name,
                            from_full_path, to_parent_handle, 
                            to_component_name, to_full_path, 
                            from_parent_hint, to_parent_hint,
                            op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }
        
   int SyncClient::symlink(const zoidfs::zoidfs_handle_t *from_parent_handle,
                            const char *from_component_name,
                            const char *from_full_path,
                            const zoidfs::zoidfs_handle_t *to_parent_handle,
                            const char *to_component_name,
                            const char *to_full_path,
                            const zoidfs::zoidfs_sattr_t *sattr,
                            zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                            zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.isymlink (&req, from_parent_handle, from_component_name,
                                 from_full_path, to_parent_handle, 
                                 to_component_name, to_full_path,
                                 sattr, from_parent_hint, to_parent_hint,
                                 op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }
   
                      
   int SyncClient::mkdir(const zoidfs::zoidfs_handle_t *parent_handle,
                          const char *component_name, const char *full_path,
                          const zoidfs::zoidfs_sattr_t *sattr,
                          zoidfs::zoidfs_cache_hint_t *parent_hint,
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.imkdir(&req,parent_handle, component_name, full_path,
                             sattr, parent_hint, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }


   int SyncClient::readdir(const zoidfs::zoidfs_handle_t *parent_handle,
                            zoidfs::zoidfs_dirent_cookie_t cookie, 
                            size_t *entry_count_,
                            zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                            zoidfs::zoidfs_cache_hint_t *parent_hint,
                            zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req; 
      int ret = asclient_.ireaddir(&req, parent_handle, cookie, entry_count_, entries, 
                               flags, parent_hint, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

   int SyncClient::resize(const zoidfs::zoidfs_handle_t *handle, 
                           zoidfs::zoidfs_file_size_t size,
                           zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.iresize(&req, handle, size, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }


   int SyncClient::read(const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                         void *mem_starts[], const size_t mem_sizes[],
                         size_t file_count, 
                         const zoidfs::zoidfs_file_ofs_t file_starts[],
                         zoidfs::zoidfs_file_size_t file_sizes[],
                         zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.iread(&req, handle, mem_count, mem_starts, mem_sizes, file_count,
                                file_starts, file_sizes, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

   int SyncClient::write(const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                          const void *mem_starts[], const size_t mem_sizes[],
                          size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                          zoidfs::zoidfs_file_ofs_t file_sizes[],
                          zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.iwrite(&req, handle, mem_count, mem_starts, mem_sizes, 
                             file_count, file_starts, file_sizes, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret); 
   }

   int SyncClient::init (zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.iinit ( &req, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret);       
   }
   
   int SyncClient::finalize (zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.ifinalize ( &req, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret);     
   }

   int SyncClient::null ( zoidfs::zoidfs_op_hint_t * op_hint)
   {
      zoidfs_request_t req;
      int ret = asclient_.inull ( &req, op_hint);
      return (ret == ZFS_OK ? waitOp (req, ZFS_COMP_DONE) : ret);     
   }

   /**
    * Wait until r completes and return the operation's return code.
    * Releases r
    */
   int SyncClient::waitOp (zoidfs_request_t r, zoidfs_comp_mask_t mask)
   {
      // Automatically release r, no matter what
      ScopedRequest req (r);

      int ret = asclient_.request_test (req, ZOIDFS_TIMEOUT_INF, mask);
      if (ret != ZFS_OK)
         return ret;

      int status;
      ret = asclient_.request_get_error (req, &status);
      fprintf (stderr, "RETURN: %i\n",ret);
      // THis is now incorrect, this should check the client via some other means
      // get_error should never fail?
      //ASSERT(ret == ZFS_OK);

      return ZFS_OK;
   }

   //========================================================================
}
