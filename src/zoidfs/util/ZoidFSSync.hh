#ifndef ZOIDFS_ZOIDFSSYNC_HH
#define ZOIDFS_ZOIDFSSYNC_HH

#include "ZoidFSAPI.hh"
#include "zoidfs-wrapped.hh"

namespace zoidfs
{
//===========================================================================

class ZoidFSSync
{

public:

      virtual inline int init(void) ;

      virtual inline int finalize(void) ;

      virtual inline int null(void) ;

      virtual inline int getattr(const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr) ;

      virtual inline int setattr(const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * sattr,
            zoidfs_attr_t * attr) ;

      virtual inline int lookup(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * handle) ;

      virtual inline int readlink(const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length) ;

      virtual inline int read(const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const uint64_t file_starts[],
            uint64_t file_sizes[]) ;

      virtual inline int write(const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const uint64_t file_starts[],
            uint64_t file_sizes[]) ;

      virtual inline int commit(const zoidfs_handle_t * handle) ;

      virtual inline int create(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created) ;

      virtual inline int remove(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint) ;

      virtual inline int rename(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint) ;

      virtual inline int link(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint) ;


      virtual inline int symlink(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint) ;

      virtual inline int mkdir(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint) ;

      virtual inline int readdir(const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint) ;

      virtual inline int resize(const zoidfs_handle_t * handle,
            uint64_t size) ; 


   virtual ~ZoidFSSync (); 
}; 

//===========================================================================

inline int ZoidFSSync::init(void)
{
   return zoidfs::zoidfs_init (); 
}

inline int ZoidFSSync::finalize(void)
{
   return zoidfs::zoidfs_finalize (); 
}

inline int ZoidFSSync::null(void)
{
   return zoidfs::zoidfs_null (); 
}

inline int ZoidFSSync::getattr(const zoidfs_handle_t * handle,
   zoidfs_attr_t * attr) 
{
   return zoidfs::zoidfs_getattr (handle, attr); 
}

inline int ZoidFSSync::setattr(const zoidfs_handle_t * handle,
   const zoidfs_sattr_t * sattr,
   zoidfs_attr_t * attr)
{
   return zoidfs::zoidfs_setattr (handle, sattr, attr); 
}


inline int ZoidFSSync::lookup(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   zoidfs_handle_t * handle)
{
   return zoidfs::zoidfs_lookup (parent_handle, component_name, full_path,
         handle); 
}

inline int ZoidFSSync::readlink(const zoidfs_handle_t * handle,
   char * buffer,
   size_t buffer_length) 
{
   return zoidfs::zoidfs_readlink (handle, buffer, buffer_length); 
}

inline int ZoidFSSync::read(const zoidfs_handle_t * handle,
   size_t mem_count,
   void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const uint64_t file_starts[],
   uint64_t file_sizes[]) 
{
   return zoidfs::zoidfs_read (handle, mem_count, mem_starts, mem_sizes,
         file_count, file_starts, file_sizes); 
}

inline int ZoidFSSync::write(const zoidfs_handle_t * handle,
   size_t mem_count,
   const void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const uint64_t file_starts[],
   uint64_t file_sizes[]) 
{
   return zoidfs::zoidfs_write (handle, mem_count, mem_starts,
         mem_sizes, file_count, file_starts, file_sizes); 
}

inline int ZoidFSSync::commit(const zoidfs_handle_t * handle)
{
   return zoidfs::zoidfs_commit (handle); 
}

inline int ZoidFSSync::create(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   const zoidfs_sattr_t * attr,
   zoidfs_handle_t * handle,
   int * created)
{
   return zoidfs::zoidfs_create (parent_handle, component_name,
         full_path, attr, handle, created); 
}

inline int ZoidFSSync::remove(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   zoidfs_cache_hint_t * parent_hint) 
{
   return zoidfs::zoidfs_remove (parent_handle, component_name, full_path,
         parent_hint); 
}

inline int ZoidFSSync::rename(const zoidfs_handle_t * from_parent_handle,
   const char * from_component_name,
   const char * from_full_path,
   const zoidfs_handle_t * to_parent_handle,
   const char * to_component_name,
   const char * to_full_path,
   zoidfs_cache_hint_t * from_parent_hint,
   zoidfs_cache_hint_t * to_parent_hint)
{
   return zoidfs::zoidfs_rename (from_parent_handle, from_component_name, 
         from_full_path, to_parent_handle, to_component_name, to_full_path,
         from_parent_hint, to_parent_hint); 
}

inline int ZoidFSSync::link(const zoidfs_handle_t * from_parent_handle,
   const char * from_component_name,
   const char * from_full_path,
   const zoidfs_handle_t * to_parent_handle,
   const char * to_component_name,
   const char * to_full_path,
   zoidfs_cache_hint_t * from_parent_hint,
   zoidfs_cache_hint_t * to_parent_hint)
{
   return zoidfs::zoidfs_link (from_parent_handle, 
         from_component_name, from_full_path,
         to_parent_handle, to_component_name, to_full_path, from_parent_hint,
         to_parent_hint); 
}


inline int ZoidFSSync::symlink(const zoidfs_handle_t * from_parent_handle,
   const char * from_component_name,
   const char * from_full_path,
   const zoidfs_handle_t * to_parent_handle,
   const char * to_component_name,
   const char * to_full_path,
   const zoidfs_sattr_t * attr,
   zoidfs_cache_hint_t * from_parent_hint,
   zoidfs_cache_hint_t * to_parent_hint)
{
   return zoidfs::zoidfs_symlink (from_parent_handle,
         from_component_name, from_full_path, to_parent_handle,
         to_component_name, to_full_path, attr, from_parent_hint,
         to_parent_hint); 
}

inline int ZoidFSSync::mkdir(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   const zoidfs_sattr_t * attr,
   zoidfs_cache_hint_t * parent_hint)
{
   return zoidfs::zoidfs_mkdir (parent_handle, component_name, 
         full_path, attr, parent_hint); 

}

inline int ZoidFSSync::readdir(const zoidfs_handle_t * parent_handle,
   zoidfs_dirent_cookie_t cookie,
   size_t * entry_count,
   zoidfs_dirent_t * entries,
   uint32_t flags,
   zoidfs_cache_hint_t * parent_hint)
{
   return zoidfs::zoidfs_readdir (parent_handle, cookie, 
         entry_count, entries, flags, parent_hint); 
}

inline int ZoidFSSync::resize(const zoidfs_handle_t * handle,
   uint64_t size) 
{
   return zoidfs::zoidfs_resize (handle, size); 
}



//===========================================================================
}

#endif
