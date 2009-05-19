#ifndef CLIENT_IOFWDCLIENT_HH
#define CLIENT_IOFWDCLIENT_HH

#include "zoidfs/ZoidFSAPI.hh"
#include "CommChannel.hh"

namespace client
{
   //========================================================================
   //========================================================================
   
   class IOFWDClient : public zoidfs::ZoidFSAPI
   {

      public:
         IOFWDClient (const char * iofwdhost); 

      public:
         virtual int zoidfs_init (); 

         virtual int zoidfs_finalize (); 

         virtual int zoidfs_null (); 


         virtual int zoidfs_getattr(const zoidfs::zoidfs_handle_t * handle ,
               zoidfs::zoidfs_attr_t * attr );

         virtual int zoidfs_setattr(const zoidfs::zoidfs_handle_t * handle ,
               const zoidfs::zoidfs_sattr_t * sattr ,
               zoidfs::zoidfs_attr_t * attr );

         virtual int zoidfs_lookup(const zoidfs::zoidfs_handle_t * parent_handle ,
               const char * component_name ,
               const char * full_path ,
               zoidfs::zoidfs_handle_t * handle );

         virtual int zoidfs_readlink(const zoidfs::zoidfs_handle_t * handle ,
               char * buffer ,
               size_t buffer_length );

         virtual int zoidfs_read(const zoidfs::zoidfs_handle_t * handle ,
               size_t mem_count ,
               void * mem_starts[] ,
               const size_t mem_sizes[] ,
               size_t file_count ,
               const uint64_t file_starts[] ,
               uint64_t file_sizes[] );

         virtual int zoidfs_write(const zoidfs::zoidfs_handle_t * handle ,
               size_t mem_count ,
               const void * mem_starts[] ,
               const size_t mem_sizes[] ,
               size_t file_count ,
               const uint64_t file_starts[] ,
               uint64_t file_sizes[] );

         virtual int zoidfs_commit(const zoidfs::zoidfs_handle_t * handle );


         virtual int zoidfs_create(const zoidfs::zoidfs_handle_t * parent_handle ,
               const char * component_name ,
               const char * full_path ,
               const zoidfs::zoidfs_sattr_t * attr ,
               zoidfs::zoidfs_handle_t * handle ,
               int * created );


         virtual int zoidfs_remove(const zoidfs::zoidfs_handle_t * parent_handle ,
               const char * component_name ,
               const char * full_path ,
               zoidfs::zoidfs_cache_hint_t * parent_hint );

         virtual int zoidfs_rename(const zoidfs::zoidfs_handle_t * from_parent_handle ,
               const char * from_component_name ,
               const char * from_full_path ,
               const zoidfs::zoidfs_handle_t * to_parent_handle ,
               const char * to_component_name ,
               const char * to_full_path ,
               zoidfs::zoidfs_cache_hint_t * from_parent_hint ,
               zoidfs::zoidfs_cache_hint_t * to_parent_hint );


         virtual int zoidfs_link(const zoidfs::zoidfs_handle_t * from_parent_handle ,
               const char * from_component_name ,
               const char * from_full_path ,
               const zoidfs::zoidfs_handle_t * to_parent_handle ,
               const char * to_component_name ,
               const char * to_full_path ,
               zoidfs::zoidfs_cache_hint_t * from_parent_hint ,
               zoidfs::zoidfs_cache_hint_t * to_parent_hint );


         virtual int zoidfs_symlink(const zoidfs::zoidfs_handle_t * from_parent_handle ,
               const char * from_component_name ,
               const char * from_full_path ,
               const zoidfs::zoidfs_handle_t * to_parent_handle ,
               const char * to_component_name ,
               const char * to_full_path ,
               const zoidfs::zoidfs_sattr_t * attr ,
               zoidfs::zoidfs_cache_hint_t * from_parent_hint ,
               zoidfs::zoidfs_cache_hint_t * to_parent_hint );

         virtual int zoidfs_mkdir(const zoidfs::zoidfs_handle_t * parent_handle ,
               const char * component_name ,
               const char * full_path ,
               const zoidfs::zoidfs_sattr_t * attr ,
               zoidfs::zoidfs_cache_hint_t * parent_hint );

         virtual int zoidfs_readdir(const zoidfs::zoidfs_handle_t * parent_handle ,
               zoidfs::zoidfs_dirent_cookie_t cookie ,
               size_t * entry_count ,
               zoidfs::zoidfs_dirent_t * entries ,
               uint32_t flags ,
               zoidfs::zoidfs_cache_hint_t * parent_hint ); 

         virtual int zoidfs_resize(const zoidfs::zoidfs_handle_t * handle ,
               uint64_t size );


      public:
         virtual ~IOFWDClient (); 


      protected:
         CommChannel comm_; 



   }; 
   
   //========================================================================
   //========================================================================
}

#endif
