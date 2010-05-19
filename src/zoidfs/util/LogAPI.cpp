#include <boost/format.hpp>
#include <numeric>

#include "iofwdutil/tools.hh"
#include "zoidfs-util.hh"
#include "iofwdutil/tools.hh"
#include "LogAPI.hh"
#include "iofwdutil/Factory.hh"
#include "iofwdutil/LinkHelper.hh"


GENERIC_FACTORY_CLIENT(std::string,
      zoidfs::util::ZoidFSAPI,
      zoidfs::util::LogAPI,
      "log",
      log);


using namespace boost;

namespace zoidfs
{
   namespace util
   {
//===========================================================================

   LogAPI::LogAPI () : opcounter_(0)
   {
   }

   LogAPI::~LogAPI ()
   {
   }

//===========================================================================


#define LOG(a) ZLOG_DEBUG((*log_),a)

void LogAPI::configure (const iofwdutil::ConfigFile & config)
{
   std::string logsource = config.getKeyDefault ("logname", "LogAPI");
   std::string apiname = config.getKeyDefault ("blocking_api", "zoidfs");

   log_ = &iofwdutil::IOFWDLog::getSource (logsource.c_str());
   ZLOG_INFO((*log_), format("Using log source name '%s'") % logsource);
   ZLOG_INFO((*log_), format("Using zoidfs blocking API '%s'") % apiname);
   api_.reset (iofwdutil::Factory<
                        std::string,
                        zoidfs::util::ZoidFSAPI>::construct(apiname)());
   iofwdutil::Configurable::configure_if_needed (api_.get(),
         config.openSectionDefault(apiname.c_str()));
}

int LogAPI::init()
{
   LOG("zoidfs_init");
   int ret = api_->init();
   checkerror(ret);
   return ret;
}

int LogAPI::finalize(void)
{
   LOG("zoidfs_finalize");
   int ret = api_->finalize ();
   checkerror(ret);
   return ret;
}

int LogAPI::null(void)
{
   LOG("zoidfs_null");
   int ret =  api_->null ();
   checkerror(ret);
   return ret;
}

int LogAPI::getattr(const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_getattr handle=%s") % handle2string(handle));
   int ret = api_->getattr (handle, attr, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::setattr(const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * a1,
            zoidfs_attr_t * a2,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_setattr handle=%s") % handle2string(handle));
   int ret = api_->setattr (handle, a1, a2, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::lookup(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * h,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_lookup %s") % filespec2string (parent_handle,
            component_name, full_path));
   int ret = api_->lookup (parent_handle, component_name, full_path, h, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::readlink(const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_readlink %s buf=%p len=%i") % handle2string(handle)
         % (void*) buffer % buffer_length);
   int ret = api_->readlink (handle, buffer, buffer_length, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::read(const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * op_hint)
{
   int opid = opcounter_.fetch_and_incr();
   LOG(format("zoidfs_read (opid %i) %s mem_count=%u file_count=%u bytes=%lu")
         % opid % handle2string(handle) % mem_count % file_count
         % std::accumulate (&mem_sizes[0], &mem_sizes[mem_count], 0));
   int ret = api_->read (handle, mem_count, mem_starts, mem_sizes,
         file_count, file_starts, file_sizes, op_hint);
   LOG(format("zoidfs_read: (opid %i) ret=%i bytes_read=%lu")
         % opid % ret % std::accumulate (&file_sizes[0],
            &file_sizes[file_count], 0));
   checkerror(ret);
   return ret;
}

int LogAPI::write(const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            const zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * op_hint)
{
   int opid = opcounter_.fetch_and_incr ();
   LOG(format("zoidfs_write (op %i) %s mem_count=%u file_count=%u bytes=%lu")
         % opid % handle2string(handle) % mem_count % file_count
         % std::accumulate (&mem_sizes[0], &mem_sizes[mem_count], 0));
   int ret = api_->write (handle, mem_count, mem_starts,
         mem_sizes, file_count, file_starts, file_sizes, op_hint);
   LOG(format("zoidfs_write: (op %i) ret=%i bytes_written=%lu")
         % opid % ret % std::accumulate (&file_sizes[0],
            &file_sizes[file_count], 0));
   checkerror(ret);
   return ret;
}

int LogAPI::commit(const zoidfs_handle_t * handle,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_commit %s") % handle2string(handle));
   int ret = api_->commit (handle, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::create(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_create"));
   int ret= api_->create (parent_handle, component_name, full_path,
         attr, handle, created, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::remove(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_remove"));
   int ret= api_->remove (parent_handle, component_name, full_path,
         parent_hint, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::rename(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_rename"));
   int ret= api_->rename (from_parent_handle, from_component_name,
         from_full_path, to_parent_handle, to_component_name, to_full_path,
         from_parent_hint, to_parent_hint, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::link(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * op_hint)
{
   LOG("zoidfs_symlink");
   int ret = api_->link (from_parent_handle, from_component_name,
         from_full_path, to_parent_handle, to_component_name,
         to_full_path, from_parent_hint, to_parent_hint, op_hint);
   checkerror(ret);
   return ret;
}


int LogAPI::symlink(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * op_hint)
{
   LOG("zoidfs_symlink");
   int ret = api_->symlink (from_parent_handle, from_component_name,
         from_full_path, to_parent_handle, to_component_name,
         to_full_path, attr, from_parent_hint, to_parent_hint, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::mkdir(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * op_hint)
{
   LOG("zoidfs_mkdir");
   int ret= api_->mkdir (parent_handle, component_name, full_path,
         attr, parent_hint, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::readdir(const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_readdir entry_count=%u") % *entry_count);
   int ret= api_->readdir (parent_handle, cookie, entry_count, entries, flags,
         parent_hint, op_hint);
   checkerror(ret);
   return ret;
}

int LogAPI::resize(const zoidfs_handle_t * handle,
            zoidfs_file_size_t size,
            zoidfs_op_hint_t * op_hint)
{
   LOG(format("zoidfs_resize: handle=%s size=%lu")
         % handle2string(handle) % size);
   int ret=api_->resize (handle, size, op_hint);
   checkerror(ret);
   return ret;
}

void LogAPI::checkerror (int ret) const
{
   if (ret == zoidfs::ZFS_OK)
      return;

   ZLOG_DEBUG((*log_),format("!!ZoidFS call returned error!!: %s") %
         zfserror2string(ret))
}

//===========================================================================
   }

}
