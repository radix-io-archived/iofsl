#include <boost/format.hpp>
#include "zoidfs-util.hh"
#include "iofwdutil/tools.hh"
#include "LogAPI.hh"

using namespace boost; 

namespace zoidfs
{
//===========================================================================

   LogAPI::LogAPI (const char * name, ZoidFSAPI * api)
      : log_ (iofwdutil::IOFWDLog::getSource (name)),
        api_ (api ? api : &fallback_) 
   {
   }

   LogAPI::~LogAPI ()
   {
   }

//===========================================================================

#define LOG(a) ZLOG_INFO(log_,a)

int LogAPI::init(void)
{
   LOG("zoidfs_init"); 
   return api_->init (); 
}

int LogAPI::finalize(void)
{
   LOG("zoidfs_finalize"); 
   return api_->finalize (); 
}

int LogAPI::null(void)
{
   LOG("zoidfs_null"); 
   return api_->null (); 
}

int LogAPI::getattr(const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr)
{
   LOG(format("zoidfs_getattr handle=%s") % handle2string(handle)); 
   return api_->getattr (handle, attr); 
}

int LogAPI::setattr(const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * a1,
            zoidfs_attr_t * a2)
{
   LOG(format("zoidfs_setattr handle=%s") % handle2string(handle)); 
   return api_->setattr (handle, a1, a2); 
}

int LogAPI::lookup(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * h)
{
   LOG(format("zoidfs_lookup %s") % filespec2string (parent_handle,
            component_name, full_path)); 
   return api_->lookup (parent_handle, component_name, full_path, h); 
}

int LogAPI::readlink(const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length)
{
   LOG(format("zoidfs_readlink %s buf=%p len=%i") % handle2string(handle)
         % buffer % buffer_length); 
   return api_->readlink (handle, buffer, buffer_length); 
}

int LogAPI::read(const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const uint64_t file_starts[],
            uint64_t file_sizes[])
{
   LOG(format("zoidfs_read %s") % handle2string(handle)); 
   return api_->read (handle, mem_count, mem_starts, mem_sizes, 
         file_count, file_starts, file_sizes); 
}

int LogAPI::write(const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const uint64_t file_starts[],
            uint64_t file_sizes[])
{
   LOG(format("zoidfs_write %s") % handle2string(handle)); 
   return api_->write (handle, mem_count, mem_starts, 
         mem_sizes, file_count, file_starts, file_sizes); 

}

int LogAPI::commit(const zoidfs_handle_t * handle)
{
   LOG(format("zoidfs_commit %s") % handle2string(handle)); 
   return api_->commit (handle);
}

int LogAPI::create(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created)
{
   LOG(format("zoidfs_create")); 
   return api_->create (parent_handle, component_name, full_path, 
         attr, handle, created); 
}

int LogAPI::remove(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint)
{
   LOG(format("zoidfs_remove")); 
   return api_->remove (parent_handle, component_name, full_path,
         parent_hint); 
}

int LogAPI::rename(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint)
{
   LOG(format("zoidfs_rename")); 
   return api_->rename (from_parent_handle, from_component_name, 
         from_full_path, to_parent_handle, to_component_name, to_full_path,
         from_parent_hint, to_parent_hint); 
}

int LogAPI::link(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint)
{
   LOG("zoidfs_symlink"); 
   return api_->link (from_parent_handle, from_component_name, 
         from_full_path, to_parent_handle, to_component_name, 
         to_full_path, from_parent_hint, to_parent_hint);
}


int LogAPI::symlink(const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint)
{
   LOG("zoidfs_symlink");
   return api_->symlink (from_parent_handle, from_component_name,
         from_full_path, to_parent_handle, to_component_name, 
         to_full_path, attr, from_parent_hint, to_parent_hint);
}

int LogAPI::mkdir(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint)
{
   LOG("zoidfs_mkdir"); 
   return api_->mkdir (parent_handle, component_name, full_path, 
         attr, parent_hint);
}

int LogAPI::readdir(const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint)
{
   LOG(format("zoidfs_readdir entry_count=%u") % *entry_count); 
   return api_->readdir (parent_handle, cookie, entry_count, entries, flags,
         parent_hint); 
}

int LogAPI::resize(const zoidfs_handle_t * handle,
            uint64_t size)
{
   LOG("zoidfs_resize"); 
   return api_->resize (handle, size); 
} 



//===========================================================================

}
