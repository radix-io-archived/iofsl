#include <vector>
#include <boost/format.hpp>

#include "ZoidFSSync.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/IOFWDLog.hh"


GENERIC_FACTORY_CLIENT(std::string,
      zoidfs::util::ZoidFSAPI,
      zoidfs::util::ZoidFSSync,
      "zoidfs",
      zoidfs);


namespace zoidfs
{
   namespace util
   {
//===========================================================================

ZoidFSSync::ZoidFSSync ()
   : log_(iofwdutil::IOFWDLog::getSource("zoidfssync"))
{
}

int ZoidFSSync::init()
{
   /* init zoidfs */
   return zoidfs::zoidfs_init();
}

void ZoidFSSync::configure (const iofwdutil::ConfigFile & config)
{
   std::vector<std::string> handlers;

   try
   {
      handlers = config.getMultiKey ("handlers");
   }
   catch (const iofwdutil::CFKeyMissingException & )
   {
      ZLOG_ERROR(log_, "No zoidfs handlers specified! Defaulting to 'local'");
      handlers.push_back ("LOCAL");
   }

   /* setup the requested handlers */
   std::vector<const char *> chandlers (handlers.size());
   ZLOG_DEBUG(log_, "Using handlers:");
   for (size_t i=0; i<handlers.size(); ++i)
   {
      ZLOG_DEBUG(log_, boost::format("%i: %s") % (i+1) % handlers[i]);
      chandlers[i] = handlers[i].c_str();
   }

   zoidfs::zint_setup_handlers(chandlers.size(), &chandlers[0]);

   /* allow the handlers to setup themselves based on config options */
   zoidfs::zint_set_handler_options(config.getConfigHandle(), config.getSectionHandle());
}

int ZoidFSSync::finalize(void)
{
   return zoidfs::zoidfs_finalize ();
}

int ZoidFSSync::null(void)
{
   return zoidfs::zoidfs_null ();
}

int ZoidFSSync::getattr(const zoidfs_handle_t * handle,
   zoidfs_attr_t * attr,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_getattr (handle, attr, op_hint);
}

int ZoidFSSync::setattr(const zoidfs_handle_t * handle,
   const zoidfs_sattr_t * sattr,
   zoidfs_attr_t * attr,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_setattr (handle, sattr, attr, op_hint);
}


int ZoidFSSync::lookup(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   zoidfs_handle_t * handle,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_lookup (parent_handle, component_name, full_path,
         handle, op_hint);
}

int ZoidFSSync::readlink(const zoidfs_handle_t * handle,
   char * buffer,
   size_t buffer_length,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_readlink (handle, buffer, buffer_length, op_hint);
}

int ZoidFSSync::read(const zoidfs_handle_t * handle,
   size_t mem_count,
   void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const zoidfs_file_ofs_t file_starts[],
   const zoidfs_file_size_t file_sizes[],
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_read (handle, mem_count, mem_starts, mem_sizes,
         file_count, file_starts, file_sizes, op_hint);
}

int ZoidFSSync::write(const zoidfs_handle_t * handle,
   size_t mem_count,
   const void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const zoidfs_file_ofs_t file_starts[],
   const zoidfs_file_size_t file_sizes[],
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_write (handle, mem_count, mem_starts,
         mem_sizes, file_count, file_starts, file_sizes, op_hint);
}

int ZoidFSSync::commit(const zoidfs_handle_t * handle,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_commit (handle, op_hint);
}

int ZoidFSSync::create(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   const zoidfs_sattr_t * attr,
   zoidfs_handle_t * handle,
   int * created,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_create (parent_handle, component_name,
         full_path, attr, handle, created, op_hint);
}

int ZoidFSSync::remove(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   zoidfs_cache_hint_t * parent_hint,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_remove (parent_handle, component_name, full_path,
         parent_hint, op_hint);
}

int ZoidFSSync::rename(const zoidfs_handle_t * from_parent_handle,
   const char * from_component_name,
   const char * from_full_path,
   const zoidfs_handle_t * to_parent_handle,
   const char * to_component_name,
   const char * to_full_path,
   zoidfs_cache_hint_t * from_parent_hint,
   zoidfs_cache_hint_t * to_parent_hint,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_rename (from_parent_handle, from_component_name,
         from_full_path, to_parent_handle, to_component_name, to_full_path,
         from_parent_hint, to_parent_hint, op_hint);
}

int ZoidFSSync::link(const zoidfs_handle_t * from_parent_handle,
   const char * from_component_name,
   const char * from_full_path,
   const zoidfs_handle_t * to_parent_handle,
   const char * to_component_name,
   const char * to_full_path,
   zoidfs_cache_hint_t * from_parent_hint,
   zoidfs_cache_hint_t * to_parent_hint,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_link (from_parent_handle,
         from_component_name, from_full_path,
         to_parent_handle, to_component_name, to_full_path, from_parent_hint,
         to_parent_hint, op_hint);
}


int ZoidFSSync::symlink(const zoidfs_handle_t * from_parent_handle,
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
   return zoidfs::zoidfs_symlink (from_parent_handle,
         from_component_name, from_full_path, to_parent_handle,
         to_component_name, to_full_path, attr, from_parent_hint,
         to_parent_hint, op_hint);
}

int ZoidFSSync::mkdir(const zoidfs_handle_t * parent_handle,
   const char * component_name,
   const char * full_path,
   const zoidfs_sattr_t * attr,
   zoidfs_cache_hint_t * parent_hint,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_mkdir (parent_handle, component_name,
         full_path, attr, parent_hint, op_hint);

}

int ZoidFSSync::readdir(const zoidfs_handle_t * parent_handle,
   zoidfs_dirent_cookie_t cookie,
   size_t * entry_count,
   zoidfs_dirent_t * entries,
   uint32_t flags,
   zoidfs_cache_hint_t * parent_hint,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_readdir (parent_handle, cookie,
         entry_count, entries, flags, parent_hint, op_hint);
}

int ZoidFSSync::resize(const zoidfs_handle_t * handle,
   zoidfs_file_size_t size,
   zoidfs_op_hint_t * op_hint)
{
   return zoidfs::zoidfs_resize (handle, size, op_hint);
}


ZoidFSSync::~ZoidFSSync ()
{
}

//===========================================================================
    }
}
