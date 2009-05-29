#ifndef ZOIDFS_NULLAPI_HH
#define ZOIDFS_NULLAPI_HH

#include "ZoidFSAPI.hh"
#include "zoidfs-wrapped.hh"

namespace zoidfs
{

   /**
    * Just returns not implemented for every call
    */
   class NullAPI : public ZoidFSAPI 
   {

      public:
         NullAPI (); 

         virtual ~NullAPI (); 

      public:

         virtual int init(void)
         { return ZFS_OK; }

         virtual int finalize(void)
         { return ZFS_OK; } 

         virtual int null(void)
         { return ZFSERR_NOTIMPL; } 

         virtual int getattr(const zoidfs_handle_t * ,
               zoidfs_attr_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int setattr(const zoidfs_handle_t * ,
               const zoidfs_sattr_t * ,
               zoidfs_attr_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int lookup(const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               zoidfs_handle_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int readlink(const zoidfs_handle_t * ,
               char * ,
               size_t )
         { return ZFSERR_NOTIMPL; }

         virtual int read(const zoidfs_handle_t * ,
               size_t ,
               void * [],
               const size_t [],
               size_t ,
               const uint64_t [],
               uint64_t [])
         { return ZFSERR_NOTIMPL; }

         virtual int write(const zoidfs_handle_t * ,
               size_t ,
               const void * [],
               const size_t [],
               size_t ,
               const uint64_t [],
               uint64_t [])
         { return ZFSERR_NOTIMPL; }

         virtual int commit(const zoidfs_handle_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int create(const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               const zoidfs_sattr_t * ,
               zoidfs_handle_t * ,
               int * )
         { return ZFSERR_NOTIMPL; }

         virtual int remove(const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               zoidfs_cache_hint_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int rename(const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               zoidfs_cache_hint_t * ,
               zoidfs_cache_hint_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int link(const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               zoidfs_cache_hint_t * ,
               zoidfs_cache_hint_t * )
         { return ZFSERR_NOTIMPL; }


         virtual int symlink(const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               const zoidfs_sattr_t * ,
               zoidfs_cache_hint_t * ,
               zoidfs_cache_hint_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int mkdir(const zoidfs_handle_t * ,
               const char * ,
               const char * ,
               const zoidfs_sattr_t * ,
               zoidfs_cache_hint_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int readdir(const zoidfs_handle_t * ,
               zoidfs_dirent_cookie_t ,
               size_t * ,
               zoidfs_dirent_t * ,
               uint32_t ,
               zoidfs_cache_hint_t * )
         { return ZFSERR_NOTIMPL; }

         virtual int resize(const zoidfs_handle_t * ,
               uint64_t )
         { return ZFSERR_NOTIMPL; } 



   };
}

#endif
