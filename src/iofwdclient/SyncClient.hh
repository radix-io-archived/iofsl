#ifndef IOFWDCLIENT_SYNCCLIENT_HH
#define IOFWDCLIENT_SYNCCLIENT_HH

#include "zoidfs/zoidfs.h"

#include "iofwdclient/iofwdclient-fwd.hh"
#include "iofwdclient/ASClient.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwd/RPCClient.hh"

#include <boost/shared_ptr.hpp>
namespace iofwdclient
{
   //========================================================================

   /**
    * Implements zoidfs normal API on top of the async API
    */
   class SyncClient
   {
      public:
         // this is zoidfs_init
         SyncClient (iofwdutil::IOFWDLogSource & log, ASClient & asclient);

         // this is zoidfs_finalize
         ~SyncClient ();

         // ------------- blocking ZoidFS functions -------------------------

         int getattr (const zoidfs::zoidfs_handle_t * handle,
               zoidfs::zoidfs_attr_t * attr,
               zoidfs::zoidfs_op_hint_t * op_hint);

         int setattr(const zoidfs::zoidfs_handle_t *handle,
                     const zoidfs::zoidfs_sattr_t *sattr,
                     zoidfs::zoidfs_attr_t *attr, 
                     zoidfs::zoidfs_op_hint_t * op_hint);

         int lookup(const zoidfs::zoidfs_handle_t *parent_handle,
                    const char *component_name, 
                    const char *full_path,
                    zoidfs::zoidfs_handle_t *handle,
                    zoidfs::zoidfs_op_hint_t * op_hint);

         int readlink(const zoidfs::zoidfs_handle_t *handle, 
                      char *buffer,
                      size_t buffer_length,
                      zoidfs::zoidfs_op_hint_t * op_hint);
                      
         int commit(const zoidfs::zoidfs_handle_t *handle,
                    zoidfs::zoidfs_op_hint_t * op_hint);

         int create(const zoidfs::zoidfs_handle_t *parent_handle,
                    const char *component_name, 
                    const char *full_path,
                    const zoidfs::zoidfs_sattr_t *sattr, 
                    zoidfs::zoidfs_handle_t *handle,
                    int *created,
                    zoidfs::zoidfs_op_hint_t * op_hint);

                    
         int remove(const zoidfs::zoidfs_handle_t *parent_handle,
                    const char *component_name, const char *full_path,
                    zoidfs::zoidfs_cache_hint_t *parent_hint,
                    zoidfs::zoidfs_op_hint_t * op_hint);                    

                     
         int rename(const zoidfs::zoidfs_handle_t *from_parent_handle,
                    const char *from_component_name,
                    const char *from_full_path,
                    const zoidfs::zoidfs_handle_t *to_parent_handle,
                    const char *to_component_name,
                    const char *to_full_path,
                    zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                    zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                    zoidfs::zoidfs_op_hint_t * op_hint);
                    

         int link(const zoidfs::zoidfs_handle_t *from_parent_handle,
                  const char *from_component_name,
                  const char *from_full_path,
                  const zoidfs::zoidfs_handle_t *to_parent_handle,
                  const char *to_component_name,
                  const char *to_full_path,
                  zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                  zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                  zoidfs::zoidfs_op_hint_t * op_hint);
                   
         int symlink(const zoidfs::zoidfs_handle_t *from_parent_handle,
                      const char *from_component_name,
                      const char *from_full_path,
                      const zoidfs::zoidfs_handle_t *to_parent_handle,
                      const char *to_component_name,
                      const char *to_full_path,
                      const zoidfs::zoidfs_sattr_t *sattr,
                      zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                      zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                      zoidfs::zoidfs_op_hint_t * op_hint);

                      
         int mkdir(const zoidfs::zoidfs_handle_t *parent_handle,
                   const char *component_name, const char *full_path,
                   const zoidfs::zoidfs_sattr_t *sattr,
                   zoidfs::zoidfs_cache_hint_t *parent_hint,
                   zoidfs::zoidfs_op_hint_t * op_hint);

         int readdir(const zoidfs::zoidfs_handle_t *parent_handle,
                     zoidfs::zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                     zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                     zoidfs::zoidfs_cache_hint_t *parent_hint,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int resize(const zoidfs::zoidfs_handle_t *handle, 
                    zoidfs::zoidfs_file_size_t size,
                    zoidfs::zoidfs_op_hint_t * op_hint);


         int read(const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                  void *mem_starts[], const size_t mem_sizes[],
                  size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                  zoidfs::zoidfs_file_size_t file_sizes[],
                  zoidfs::zoidfs_op_hint_t * op_hint);

         int write(const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                   const void *mem_starts[], const size_t mem_sizes[],
                   size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                   zoidfs::zoidfs_file_ofs_t file_sizes[],
                   zoidfs::zoidfs_op_hint_t * op_hint);

         int init (zoidfs::zoidfs_op_hint_t * op_hint);
         
         int finalize (zoidfs::zoidfs_op_hint_t * op_hint);

         int null ( zoidfs::zoidfs_op_hint_t * op_hint);


      protected:

         int waitOp (zoidfs::zoidfs_request_t r, zoidfs::zoidfs_comp_mask_t);

      protected:
         iofwdutil::IOFWDLogSource & log_;
         ASClient & asclient_;
   };

   //========================================================================
}

#endif
