#ifndef ZOIDFS_SFPWRAPPER_HH
#define ZOIDFS_SFPWRAPPER_HH

#include "ZoidFSAPI.hh"
#include "zoidfs/zoidfs.h"
#include "ZoidFSSync.hh"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdutil/Configurable.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"
#include "iofwd/extraservice/sfp/SFPService.hh"

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

namespace zoidfs
{
   namespace util
   {
      //=======================================================================

      class SFPWrapper : public ZoidFSAPI, public iofwdutil::Configurable
      {
         public:
            SFPWrapper ();

            virtual ~SFPWrapper ();

         public:

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
                  zoidfs_op_hint_t * op_hint);

            virtual int lookup(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * op_hint);

            virtual int readlink(const zoidfs_handle_t * handle,
                  char * buffer,
                  size_t buffer_length,
                  zoidfs_op_hint_t * op_hint);

            virtual int read(const zoidfs_handle_t * handle,
                  size_t mem_count,
                  void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const zoidfs_file_ofs_t file_starts[],
                  const zoidfs_file_size_t file_sizes[],
                  zoidfs_op_hint_t * op_hint);

            virtual int write(const zoidfs_handle_t * handle,
                  size_t mem_count,
                  const void * mem_starts[],
                  const size_t mem_sizes[],
                  size_t file_count,
                  const zoidfs_file_ofs_t file_starts[],
                  const zoidfs_file_size_t file_sizes[],
                  zoidfs_op_hint_t * op_hint);

            virtual int commit(const zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * op_hint);

            virtual int create(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_handle_t * handle,
                  int * created,
                  zoidfs_op_hint_t * op_hint);

            virtual int remove(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * op_hint);

            virtual int rename(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * op_hint);

            virtual int link(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * op_hint);


            virtual int symlink(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * op_hint);

            virtual int mkdir(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * op_hint);

            virtual int readdir(const zoidfs_handle_t * parent_handle,
                  zoidfs_dirent_cookie_t cookie,
                  size_t * entry_count,
                  zoidfs_dirent_t * entries,
                  uint32_t flags,
                  zoidfs_cache_hint_t * parent_hint,
                  zoidfs_op_hint_t * op_hint);

            virtual int resize(const zoidfs_handle_t * handle,
                  zoidfs_file_size_t size,
                  zoidfs_op_hint_t * op_hint);

         protected:
            bool processSFPHint (const zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * op_hint, int * ret);

         protected:
            boost::scoped_ptr<ZoidFSAPI> api_;
            iofwdutil::IOFWDLogSource & log_;
            boost::shared_ptr<iofwd::extraservice::SFPService> sfp_service_;
      };

   }
}



#endif
