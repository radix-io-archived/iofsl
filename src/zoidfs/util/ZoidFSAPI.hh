#ifndef ZOIDFS_ZOIDFSAPI_HH
#define ZOIDFS_ZOIDFSAPI_HH

#include <boost/mpl/list.hpp>  // for factory
#include "zoidfs-wrapped.hh"

namespace zoidfs
{
   namespace util
   {
      //======================================================================

      /**
       * Class encapsulated interface to the ZoidFS API.
       */
      class ZoidFSAPI
      {

         public:
            // ZoidFSAPI derived classes don't take extra constructor parameters
            typedef boost::mpl::list<> FACTORY_SIGNATURE;

            virtual int init() =0;

            virtual int finalize() = 0;

            virtual int null() =0;

            virtual int getattr(const zoidfs_handle_t * handle,
                  zoidfs_attr_t * attr,
                  zoidfs_op_hint_t * op_hint) =0;

            virtual int setattr(const zoidfs_handle_t * handle,
                  const zoidfs_sattr_t * sattr,
                  zoidfs_attr_t * attr,
                  zoidfs_op_hint_t * hint) =0;

            virtual int lookup(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * hint) =0;

            virtual int readlink(const zoidfs_handle_t * handle,
                  char * buffer,
                  size_t buffer_length,
                  zoidfs_op_hint_t * hint) =0;

            virtual int read(const zoidfs_handle_t * handle,
                  size_t mem_count,
                  void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const zoidfs_file_ofs_t file_starts[],
                  zoidfs_file_size_t file_sizes[],
                  zoidfs_op_hint_t * hint) =0;

            virtual int write(const zoidfs_handle_t * handle,
                  size_t mem_count,
                  const void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const uint64_t file_starts[],
                  uint64_t file_sizes[],
                  zoidfs_op_hint_t * hint) =0;

            virtual int commit(const zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * hint) =0;

            virtual int create(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_handle_t * handle,
                  int * created,
                  zoidfs_op_hint_t * hint) =0;

            virtual int remove(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * hint) =0;

            virtual int rename(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * hint) =0;

            virtual int link(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * hint) =0;


            virtual int symlink(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * hint) =0;

            virtual int mkdir(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * hint) =0;

            virtual int readdir(const zoidfs_handle_t * parent_handle,
                  zoidfs_dirent_cookie_t cookie,
                  size_t * entry_count,
                  zoidfs_dirent_t * entries,
                  uint32_t flags,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * hint) =0;

            virtual int resize(const zoidfs_handle_t * handle,
                  uint64_t size,
                  zoidfs_op_hint_t * hint) =0;


         public:
            virtual ~ZoidFSAPI ();
      };

      //======================================================================
   }
}

#endif
