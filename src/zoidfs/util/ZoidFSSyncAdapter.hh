#ifndef ZOIDFS_UTIL_ZOIDFSSSYNCADAPTER_HH
#define ZOIDFS_UTIL_ZOIDFSSSYNCADAPTER_HH

#include <boost/scoped_ptr.hpp>
#include "zoidfs/util/ZoidFSAPI.hh"
#include "zoidfs/util/ZoidFSAsync.hh"
#include "iofwdutil/Configurable.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

namespace zoidfs
{
   namespace util
   {
      //=====================================================================

      /**
       * This class takes a zoidfs async API and transforms it into
       * a blocking API by waiting for completion of each asynchronous call.
       */
      class ZoidFSSyncAdapter : public ZoidFSAPI,
                                 public iofwdutil::Configurable
      {
         public:
            ZoidFSSyncAdapter ();

            virtual void configure (const iofwdutil::ConfigFile & config);

            virtual int init();

            virtual int finalize();

            virtual int null();

            virtual int getattr(const zoidfs_handle_t * handle,
                  zoidfs_attr_t * attr,
                  zoidfs_op_hint_t * op_hint);

            virtual int setattr(const zoidfs_handle_t * handle,
                  const zoidfs_sattr_t * sattr,
                  zoidfs_attr_t * attr,
                  zoidfs_op_hint_t * hint);

            virtual int lookup(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * hint);

            virtual int readlink(const zoidfs_handle_t * handle,
                  char * buffer,
                  size_t buffer_length,
                  zoidfs_op_hint_t * hint);

            virtual int read(const zoidfs_handle_t * handle,
                  size_t mem_count,
                  void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const zoidfs_file_ofs_t file_starts[],
                  zoidfs_file_size_t file_sizes[],
                  zoidfs_op_hint_t * hint);

            virtual int write(const zoidfs_handle_t * handle,
                  size_t mem_count,
                  const void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const zoidfs_file_ofs_t file_starts[],
                  zoidfs_file_size_t file_sizes[],
                  zoidfs_op_hint_t * hint);

            virtual int commit(const zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * hint);

            virtual int create(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_handle_t * handle,
                  int * created,
                  zoidfs_op_hint_t * hint);

            virtual int remove(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * hint);

            virtual int rename(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * hint);

            virtual int link(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * hint);


            virtual int symlink(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * hint);

            virtual int mkdir(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * hint);

            virtual int readdir(const zoidfs_handle_t * parent_handle,
                  zoidfs_dirent_cookie_t cookie,
                  size_t * entry_count,
                  zoidfs_dirent_t * entries,
                  uint32_t flags,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * hint);

            virtual int resize(const zoidfs_handle_t * handle,
                  zoidfs_file_size_t size,
                  zoidfs_op_hint_t * hint);

         protected:
            boost::scoped_ptr<ZoidFSAsync> api_;

            iofwdutil::IOFWDLogSource & log_;

      };

      //=====================================================================
   }
}

#endif
