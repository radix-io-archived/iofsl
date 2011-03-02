#ifndef IOFWDCLIENT_IOFWDCLIENT_HH
#define IOFWDCLIENT_IOFWDCLIENT_HH

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-async.h"

#include "iofwdclient/iofwdclient-fwd.hh"

#include "iofwdutil/IOFWDLog-fwd.hh"

#include <boost/scoped_ptr.hpp>

namespace iofwdclient
{
   //========================================================================

   /**
    * Implements ZoidFS API (sync + async)
    */
   class IOFWDClient
   {
      public:
         // this is zoidfs_init
         IOFWDClient ();

         // this is zoidfs_finalize
         ~IOFWDClient ();


         // -----------------------------------------------------------------
         // ------------- blocking ZoidFS functions -------------------------
         // -----------------------------------------------------------------

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
         // -----------------------------------------------------------------
         // -------------- zoidfs async methods -----------------------------
         // -----------------------------------------------------------------

         int igetattr(zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t * handle,
                   zoidfs::zoidfs_attr_t * attr,
                   zoidfs::zoidfs_op_hint_t * op_hint);

         int isetattr(zoidfs::zoidfs_request_t * request,
                      const zoidfs::zoidfs_handle_t *handle,
                      const zoidfs::zoidfs_sattr_t *sattr,
                      zoidfs::zoidfs_attr_t *attr, 
                      zoidfs::zoidfs_op_hint_t * op_hint);

         int ilookup(zoidfs::zoidfs_request_t * request,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     zoidfs::zoidfs_handle_t *handle,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int ireadlink(zoidfs::zoidfs_request_t * request,
                       const zoidfs::zoidfs_handle_t *handle,  
                       char *buffer,
                       size_t buffer_length,
                       zoidfs::zoidfs_op_hint_t * op_hint);
                       
         int icommit(zoidfs::zoidfs_request_t * request,
                     const zoidfs::zoidfs_handle_t *handle,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int icreate(zoidfs::zoidfs_request_t * request,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, 
                     const char *full_path,
                     const zoidfs::zoidfs_sattr_t *sattr, 
                     zoidfs::zoidfs_handle_t *handle,
                     int *created,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                    
         int iremove(zoidfs::zoidfs_request_t * request,
                     const zoidfs::zoidfs_handle_t *parent_handle,
                     const char *component_name, const char *full_path,
                     zoidfs::zoidfs_cache_hint_t *parent_hint,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int irename(zoidfs::zoidfs_request_t * request,
                     const zoidfs::zoidfs_handle_t *from_parent_handle,
                     const char *from_component_name,
                     const char *from_full_path,
                     const zoidfs::zoidfs_handle_t *to_parent_handle,
                     const char *to_component_name,
                     const char *to_full_path,
                     zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                     zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
         int ilink(zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t *from_parent_handle,
                   const char *from_component_name,
                   const char *from_full_path,
                   const zoidfs::zoidfs_handle_t *to_parent_handle,
                   const char *to_component_name,
                   const char *to_full_path,
                   zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                   zoidfs::zoidfs_op_hint_t * op_hint);
                   
         int isymlink(zoidfs::zoidfs_request_t * request,
                      const zoidfs::zoidfs_handle_t *from_parent_handle,
                      const char *from_component_name,
                      const char *from_full_path,
                      const zoidfs::zoidfs_handle_t *to_parent_handle,
                      const char *to_component_name,
                      const char *to_full_path,
                      const zoidfs::zoidfs_sattr_t *sattr,
                      zoidfs::zoidfs_cache_hint_t *from_parent_hint,
                      zoidfs::zoidfs_cache_hint_t *to_parent_hint,
                      zoidfs::zoidfs_op_hint_t * op_hint);
                      
         int imkdir(zoidfs::zoidfs_request_t * request,
                    const zoidfs::zoidfs_handle_t *parent_handle,
                    const char *component_name, const char *full_path,
                    const zoidfs::zoidfs_sattr_t *sattr,
                    zoidfs::zoidfs_cache_hint_t *parent_hint,
                    zoidfs::zoidfs_op_hint_t * op_hint);
                            
         int ireaddir(zoidfs::zoidfs_request_t * request,
                      const zoidfs::zoidfs_handle_t *parent_handle,
                      zoidfs::zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                      zoidfs::zoidfs_dirent_t *entries, uint32_t flags,
                      zoidfs::zoidfs_cache_hint_t *parent_hint,
                      zoidfs::zoidfs_op_hint_t * op_hint);

         int iresize(zoidfs::zoidfs_request_t * request,
                     const zoidfs::zoidfs_handle_t *handle, 
                     zoidfs::zoidfs_file_size_t size,
                     zoidfs::zoidfs_op_hint_t * op_hint);
                     
                   
         int iread(zoidfs::zoidfs_request_t * request,
                   const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                   void *mem_starts[], const size_t mem_sizes[],
                   size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                   zoidfs::zoidfs_file_size_t file_sizes[],
                   zoidfs::zoidfs_op_hint_t * op_hint);
                    
         int iwrite(zoidfs::zoidfs_request_t * request,
                    const zoidfs::zoidfs_handle_t *handle, size_t mem_count,
                    const void *mem_starts[], const size_t mem_sizes[],
                    size_t file_count, const zoidfs::zoidfs_file_ofs_t file_starts[],
                    zoidfs::zoidfs_file_ofs_t file_sizes[],
                    zoidfs::zoidfs_op_hint_t * op_hint);
                   


         // -----------------------------------------------------------------
         // ------------------ Requests & Testing ---------------------------
         // -----------------------------------------------------------------

         int request_test (zoidfs::zoidfs_request_t request,
                   zoidfs::zoidfs_timeout_t timeout,
                   zoidfs::zoidfs_comp_mask_t mask);

         int request_get_error (zoidfs::zoidfs_request_t request, int * error);

         int request_get_comp_state (zoidfs::zoidfs_request_t,
               zoidfs::zoidfs_comp_mask_t *);

         int request_free (zoidfs::zoidfs_request_t * request);

      protected:
         iofwdutil::IOFWDLogSource & log_;

         boost::scoped_ptr<PollQueue>   pollqueue_;
         boost::scoped_ptr<ASClient>    asclient_;
         boost::scoped_ptr<SyncClient>  sclient_;
   };

   //========================================================================
}

#endif
