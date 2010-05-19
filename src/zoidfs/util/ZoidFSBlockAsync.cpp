#include "ZoidFSBlockAsync.hh"

#define DOCATCH(a) \
   boost::exception_ptr e; \
   try { a } catch (...) { e = boost::current_exception (); } \
   cb (iofwdevent::CBException (e));

namespace zoidfs
{
   namespace util
   {
      //=====================================================================

      // @TODO: fix this for exception handling
      //   all calls need to get a try block and pass any exceptions caught to
      //   the callback
      
      ZoidFSBlockAsync::ZoidFSBlockAsync (ZoidFSAPI * delegate)
         : delegate_(delegate)
      {
      }

       int ZoidFSBlockAsync::init(void)
       {
          return delegate_->init ();
       }

       int ZoidFSBlockAsync::finalize(void)
       {
          return delegate_->finalize ();
       }

       void ZoidFSBlockAsync::null(const iofwdevent::CBType & cb, int * ret)
       {
          DOCATCH (*ret = delegate_->null (); )
       }

       void ZoidFSBlockAsync::getattr(const iofwdevent::CBType & cb,
             int * ret, const zoidfs_handle_t * handle,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * op_hint)
       {
          DOCATCH( *ret = delegate_->getattr (handle, attr, op_hint); )
       }

       void ZoidFSBlockAsync::setattr(const iofwdevent::CBType & cb,
             int * ret, const zoidfs_handle_t * handle,
            const zoidfs_sattr_t * sattr,
            zoidfs_attr_t * attr,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH (*ret = delegate_->setattr (handle, sattr, attr, hint);)
       }

       void ZoidFSBlockAsync::lookup(const iofwdevent::CBType & cb, int * ret,
            const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
            *ret = delegate_->lookup (parent_handle, component_name, full_path,
                handle, hint);
            )
       }

       void ZoidFSBlockAsync::readlink(const iofwdevent::CBType & cb,
             int * ret,
             const zoidfs_handle_t * handle,
            char * buffer,
            size_t buffer_length,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
          *ret = delegate_->readlink (handle, buffer, buffer_length, hint);
          )
       }

       void ZoidFSBlockAsync::read(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * handle,
            size_t mem_count,
            void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
       {
          DOCATCH (
          *ret = delegate_->read (handle, mem_count, mem_starts, mem_sizes,
                file_count, file_starts, file_sizes, hint);
          )
       }

       void ZoidFSBlockAsync::write(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * handle,
            size_t mem_count,
            const void * mem_starts[],
            const size_t mem_sizes[],
            size_t file_count,
            const zoidfs_file_ofs_t file_starts[],
            zoidfs_file_size_t file_sizes[],
            zoidfs_op_hint_t * hint)
       {
          DOCATCH (
             *ret = delegate_->write (handle, mem_count, mem_starts, mem_sizes,
                file_count, file_starts, file_sizes, hint);
          )
       }

       void ZoidFSBlockAsync::commit(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * handle,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH (
                *ret = delegate_->commit (handle, hint);
          )
       }

       void ZoidFSBlockAsync::create(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_handle_t * handle,
            int * created,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
             *ret = delegate_->create (parent_handle, component_name, full_path,
                attr, handle, created, hint);
          )
       }

       void ZoidFSBlockAsync::remove(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
             *ret = delegate_->remove (parent_handle, component_name, full_path,
                parent_hint, hint);
          )
       }

       void ZoidFSBlockAsync::rename(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
             *ret = delegate_->rename (from_parent_handle, from_component_name,
                from_full_path, to_parent_handle, to_component_name, to_full_path,
                from_parent_hint, to_parent_hint, hint);
             )
       }

       void ZoidFSBlockAsync::link(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
             *ret = delegate_->link (from_parent_handle, from_component_name,
                from_full_path, to_parent_handle, to_component_name, to_full_path,
                from_parent_hint, to_parent_hint, hint);
          )
       }


       void ZoidFSBlockAsync::symlink(const iofwdevent::CBType & cb,
             int * ret, const zoidfs_handle_t * from_parent_handle,
            const char * from_component_name,
            const char * from_full_path,
            const zoidfs_handle_t * to_parent_handle,
            const char * to_component_name,
            const char * to_full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * from_parent_hint,
            zoidfs_cache_hint_t * to_parent_hint,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
             *ret = delegate_->symlink (from_parent_handle, from_component_name,
                from_full_path, to_parent_handle, to_component_name, to_full_path,
                attr, from_parent_hint, to_parent_hint, hint);
          )
       }

       void ZoidFSBlockAsync::mkdir(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            const zoidfs_sattr_t * attr,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
            *ret = delegate_->mkdir (parent_handle, component_name, full_path,
                attr, parent_hint, hint);
          )
       }

       void ZoidFSBlockAsync::readdir(const iofwdevent::CBType & cb,
             int * ret, const zoidfs_handle_t * parent_handle,
            zoidfs_dirent_cookie_t cookie,
            size_t * entry_count,
            zoidfs_dirent_t * entries,
            uint32_t flags,
            zoidfs_cache_hint_t * parent_hint,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
            *ret = delegate_->readdir (parent_handle, cookie, entry_count,
                entries, flags, parent_hint, hint);
          )
       }

       void ZoidFSBlockAsync::resize(const iofwdevent::CBType & cb, int * ret,
             const zoidfs_handle_t * handle,
            zoidfs_file_size_t size,
            zoidfs_op_hint_t * hint)
       {
          DOCATCH(
            *ret = delegate_->resize (handle, size, hint);
          )
       }


      //=====================================================================
   }
}
