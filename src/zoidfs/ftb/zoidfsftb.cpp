#include "zoidfs/zoidfs.h"

#include "ServerSelector.hh"
#include "FTBClient.hh"

#include "iofwdutil/assert.hh"

#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <iostream>

extern "C"
{
#include <bmi.h>
}

using namespace boost::posix_time;

extern "C"
{
   // intern function of current zoidfs client
   void zoidfs_client_swap_addr(BMI_addr_t * naddr);
}

namespace zoidfs
{
   //========================================================================

   boost::scoped_ptr<ServerSelector> sel_;
   boost::scoped_ptr<FTBClient> ftb_;

   bool takeAction (int act)
   {
      bool ret;
      switch (act)
      {
         case ServerSelector::ACTION_RETURN:
            ret = false;
            break;
         case ServerSelector::ACTION_WAITRETRY:
            boost::this_thread::sleep (seconds (1));
            ret = true;
            break;
         case ServerSelector::ACTION_RETRY:
            ret = true;
            break;
         case ServerSelector::ACTION_NEWSERVER:
            {
            ret = true;
            BMI_addr_t * n = new BMI_addr_t ();
            *n = sel_->getServer ();
            zoidfs_client_swap_addr (n);
            break;
            }
         default:
            ret = false;
            ALWAYS_ASSERT(false);
      }

      return ret;
   }

   void checkAction ()
   {
      ftb_->poll ();
      takeAction (sel_->getAction ());
   }

   // Return true here will cause an immediate retry of the operation
   bool checkRet (int ret)
   {
      switch (ret)
      {
         case ZFSERR_NETWORK:
            return takeAction (sel_->reportProblem ());
         case ZFS_OK:
            return false;

         default:
            return false;
      };
      return false;
   }


   extern "C"
   {
      //======================================================================

      int zoidfs_null(void) 
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_null();
         } while (checkRet (ret));

         return ret;
      }

      int zoidfs_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_getattr(handle, attr, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_setattr
       * This function sets the attributes associated with the file handle.
       */
      int zoidfs_setattr(const zoidfs_handle_t *handle,
            const zoidfs_sattr_t *sattr, zoidfs_attr_t *attr,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_setattr(handle, sattr, attr, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_readlink
       * This function reads a symbolic link.
       */
      int zoidfs_readlink(const zoidfs_handle_t *handle, char *buffer,
            size_t buffer_length, zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_readlink(handle, buffer, buffer_length, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_lookup
       * This function returns the file handle associated with the given file
       * or directory name.
       */
      int zoidfs_lookup(const zoidfs_handle_t *parent_handle,
            const char *component_name, const char *full_path,
            zoidfs_handle_t *handle, zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_lookup(parent_handle, component_name, full_path,
                  handle, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_remove
       * This function removes the given file or directory.
       */
      int zoidfs_remove(const zoidfs_handle_t *parent_handle,
            const char *component_name, const char *full_path,
            zoidfs_cache_hint_t *parent_hint, zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_remove(parent_handle, component_name, full_path,
                  parent_hint, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_commit
       * This function flushes the buffers associated with the file handle.
       */
      int zoidfs_commit(const zoidfs_handle_t *handle,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_commit(handle, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_create
       * This function creates a new file.
       */
      int zoidfs_create(const zoidfs_handle_t *parent_handle,
            const char *component_name, const char *full_path,
            const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
            int *created, zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_create(parent_handle, component_name, full_path,
                  sattr, handle, created, op_hint);
         } while (checkRet (ret));
         return ret;
      }

      /*
       * zoidfs_rename
       * This function renames an existing file or directory.
       */
      int zoidfs_rename(const zoidfs_handle_t *from_parent_handle,
            const char *from_component_name,
            const char *from_full_path,
            const zoidfs_handle_t *to_parent_handle,
            const char *to_component_name,
            const char *to_full_path,
            zoidfs_cache_hint_t *from_parent_hint,
            zoidfs_cache_hint_t *to_parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_rename(from_parent_handle, from_component_name,
                  from_full_path, to_parent_handle, to_component_name,
                  to_full_path, from_parent_hint, to_parent_hint, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_link
       * This function creates a hard link.
       */
      int zoidfs_link(const zoidfs_handle_t *from_parent_handle,
            const char *from_component_name,
            const char *from_full_path,
            const zoidfs_handle_t *to_parent_handle,
            const char *to_component_name,
            const char *to_full_path,
            zoidfs_cache_hint_t *from_parent_hint,
            zoidfs_cache_hint_t *to_parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_link(from_parent_handle, from_component_name,
                  from_full_path, to_parent_handle, to_component_name,
                  to_full_path, from_parent_hint, to_parent_hint, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_symlink
       * This function creates a symbolic link.
       */
      int zoidfs_symlink(const zoidfs_handle_t *from_parent_handle,
            const char *from_component_name,
            const char *from_full_path,
            const zoidfs_handle_t *to_parent_handle,
            const char *to_component_name,
            const char *to_full_path,
            const zoidfs_sattr_t *sattr,
            zoidfs_cache_hint_t *from_parent_hint,
            zoidfs_cache_hint_t *to_parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_symlink(from_parent_handle, from_component_name,
                  from_full_path, to_parent_handle, to_component_name,
                  to_full_path, sattr, from_parent_hint, to_parent_hint,
                  op_hint);
         } while (checkRet (ret));
         return ret;
      }

      /*
       * zoidfs_mkdir
       * This function creates a new directory.
       */
      int zoidfs_mkdir(const zoidfs_handle_t *parent_handle,
            const char *component_name, const char *full_path,
            const zoidfs_sattr_t *sattr,
            zoidfs_cache_hint_t *parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_mkdir(parent_handle, component_name, full_path,
                  sattr, parent_hint, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_readdir
       * This function returns the dirents from the specified parent
       * directory. The cookie is a pointer which specifies where in the
       * directory to start fetching the dirents from.
       */
      int zoidfs_readdir(const zoidfs_handle_t *parent_handle,
            zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
            zoidfs_dirent_t *entries, uint32_t flags,
            zoidfs_cache_hint_t *parent_hint,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_readdir(parent_handle, cookie, entry_count_,
                  entries, flags, parent_hint, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_resize
       * This function resizes the file associated with the file handle.
       */
      int zoidfs_resize(const zoidfs_handle_t *handle, uint64_t size,
            zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_resize(handle, size, op_hint);
         } while (checkRet (ret));
         return ret;
      }

      /*
       * zoidfs_write
       * This function implements the zoidfs write call.
       */
      int zoidfs_write(const zoidfs_handle_t *handle, size_t mem_count_,
            const void *mem_starts[], const size_t mem_sizes_[],
            size_t file_count_, const zoidfs_file_ofs_t file_starts[],
            zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint)
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_write(handle, mem_count_, mem_starts, mem_sizes_,
                  file_count_, file_starts, file_sizes, op_hint);
         } while (checkRet (ret));
         return ret;
      }


      /*
       * zoidfs_read
       * This function implements the zoidfs read call.
       */
      int zoidfs_read(const zoidfs_handle_t *handle, size_t mem_count_,
            void *mem_starts[], const size_t mem_sizes_[],
            size_t file_count_, const zoidfs_file_ofs_t file_starts[],
            zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint) 
      {
         checkAction ();
         int ret = 0;
         do
         {
            ret = Pzoidfs_read(handle, mem_count_, mem_starts, mem_sizes_,
                  file_count_, file_starts, file_sizes, op_hint);
         } while (checkRet (ret));
         return ret;
      }

      /*
       * zoidfs_init
       * Initialize the client subsystems.
       */
      int zoidfs_init(void)
      {
         std::cerr << "zoidfsftb active...\n";
         sel_.reset (new ServerSelector ());
         ftb_.reset (new FTBClient (*sel_));

         int ret = 0;
         ret = Pzoidfs_init();
         return ret;
      }

      /*
       * zoidfs_finalize
       * Finalize the client subsystems.
       */
      int zoidfs_finalize(void) 
      {
         sel_.reset ();
         int ret = 0;
         ret = Pzoidfs_finalize();
         return ret;
      }


      //======================================================================
   } // extern C

   //======================================================================
} // namespace
