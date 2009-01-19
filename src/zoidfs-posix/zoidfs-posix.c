#define _LARGEFILE64_SOURCE
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include "zoidfs.h"
#include "handlecache.h"
#include "persist.h"

#define mymin(a,b) ((a)<(b) ? (a):(b))

#ifndef NDEBUG
#define zoidfs_debug(format, ...) fprintf (stderr, "zoidfs_posix: debug: " format, ##__VA_ARGS__)
#else
#define zoidfs_debug(format, ...) {}
#endif

#define zoidfs_error(format, ...) fprintf (stderr, "zoidfs_posix: error: " format, ##__VA_ARGS__)


/*
 * REMINDER:
 *  
 *     Components:
 *        handlecache  -> manages list of zoidfshandle -> open file desc
 *        persist      -> manages mapping names to zoidfs handles and
 *                        vice-versa
 */


static persist_op_t  *  persist; 



/* Convert this to thread local storage later on */
static inline const char * zoidfs_resolve_path (const zoidfs_handle_t * handle, 
      const char * component, const char * full_path)
{
   static char buf[ZOIDFS_PATH_MAX+1]; 
   int ret; 

   if (full_path)
      return full_path; 

   ret = persist_handle_to_filename (persist, handle, buf, sizeof(buf)); 
   
   if (!ret)
   {
#ifdef USE_ESTALE
      zoidfs_debug ("zoidfs_resolve_path unknown handle!\n"); 
      return 0; 
#else
      zoidfs_error ("zoidfs_resolve_path: unable to lookup handle->path!\n"); 
      exit (1);
#endif
   }

#ifndef NDEBUG
   {
      const int complen = strlen (component); 
      assert (complen + ret < ZOIDFS_PATH_MAX); 
   }
#endif
   strcpy (&buf[ret-1], component); 
   return buf; 
}


/*
 * zoidfs_null
 * This function implements a noop operation. The IOD returns a 1-byte message
 * to the CN.
 */
void zoidfs_null(void) 
{
}


/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle from
 * the ION.
 */
int zoidfs_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr) 
{
   //persist_handle_t rhandle = handle_to_internal (handle); 

   zoidfs_debug ("getattr\n"); 

    return -ENOSYS;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
int zoidfs_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t *attr) 
{
    return -ENOSYS;
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 */
int zoidfs_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length) {
    return -ENOSYS;
}

static inline int errno2zfs (int val)
{
   if (val > 0)
      return ZFS_OK; 
   return -ZFSERR_MISC; 
}

/* Try to open a file: be careful about directories and 
 * file permissions */ 
static int our_open (const char * filename, int *err)
{
   int fd; 

   *err = 0; 

   /* simple case: try to open in rdwr, no create */ 
   fd = open (filename, O_RDWR); 

   if (fd>=0)
      return fd; 


   /* ok that didn't work: try to figure out why */
   *err = errno; 

   if (*err == ENOENT)
   {
      /* file doesn't exist */ 
      return fd; 
   }

   if (*err == EACCES)
   {
      /* try in rdonly mode */
      /* No other possibilities here: the file exist, is not a directory
       * (otherwise we would get EISDIR on the previous try)
       * If we cannot open in rdonly mode we cannot open...
       */
      fd = open (filename, O_RDONLY);
      *err = errno; 
      return fd; 
   }

   /* At this point, the chance we can still open is if the file is a
    * directory (and the open fails because we tried O_RDWR) */
   if (*err != EISDIR)
      return fd; 

   /* ok so the file is a directory: we can only try to open in readonly mode
    * and hope for the best */ 
   fd = open (filename, O_RDONLY); 
   *err = errno; 
   return fd; 
}


static int our_create (const char * filename, int * err, int * created)
{
   int dummy; 
   int fd; 

   /* allow NULL for created */ 
   if (!created)
      created = &dummy; 


   while (1)
   {

      fd = open (filename, O_CREAT|O_EXCL|O_RDWR); 
      *err = errno; 
      if (*err == EEXIST)
      {
         fd = our_open (filename, err); 
         if (fd < 0 && *err == ENOENT)
         {
            /* strange: somebody removed in the mean time? */
            zoidfs_debug ("File %s disappeared before we could open?? Retrying to create\n",
                  filename); 
            continue; 
         }
         /* the file existed, so whatever reason our_open had for failing is
          * good enough for us */ 
         return fd; 
      }

      /* failed, but not because the file already existed: give up */
      return fd; 
   }
}

/* Open a file descriptor for the specified zoidfs handle; File must exist */ 
static int getfd_handle (const zoidfs_handle_t * handle)
{
   int fd; 
   char buf[ZOIDFS_PATH_MAX]; 

   /* first lookup in the cache */
   if (handlecache_lookup (handle, &fd))
      return fd; 

   /* Handle is not there: try to map to a filename and reopen */ 
   if (!persist_handle_to_filename (persist, handle, buf, sizeof(buf)))
   {
#ifdef USE_ESTALE
      zoidfs_debug ("Returned -ESTALE\n"); 
      return -ESTALE; 
#else
      zoidfs_error ("Unable to map zoidfs handle to path! Invalid handle?\n"); 
      exit (1); 
#endif
   }

   fd = open (buf, O_RDWR); 
   if (fd < 0)
      return fd;

   /* add entry to cache */ 
   handlecache_add (handle, &fd); 

   return fd; 
}

/*
 * zoidfs_lookup
 * This function returns the file handle associated with the given file or
 * directory name.
 */
int zoidfs_lookup(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_handle_t *handle) 
{
   const char * path = zoidfs_resolve_path (parent_handle, component_name, 
         full_path); 
   if (!path)
      return ZFSERR_STALE; 

   zoidfs_debug ("lookup: %s\n", path); 

   // ignore component_name
   int err; 
   int file = our_open (path, &err); 

   if (file < 0)
      return errno2zfs (err); 

   /* Lookup the persist handle for the file, creating if needed */ 
   persist_filename_to_handle (persist, full_path, handle, 1); 

   /* file openened ok, add to cache */
   handlecache_add (handle, &file); 

   return ZFS_OK; 
}


/*
 * zoidfs_remove
 * This function removes the given file or directory.
 */
int zoidfs_remove(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_cache_hint_t *parent_hint) 
{
   const char * path = zoidfs_resolve_path (parent_handle, component_name, 
         full_path); 
   // ignore component_name
   if (unlink (path)< 0) 
      return errno2zfs (errno); 

   /* remove from database */
   persist_purge (persist, path, 0); 

   /* NOTE: whoever still has the file open will still be able to access the
    * file */ 


   return ZFS_OK; 
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
int zoidfs_commit(const zoidfs_handle_t *handle) 
{
   /* file descriptor interface is not buffered... */ 
   return ZFS_OK; 
}


/*
 * zoidfs_create
 * This function creates a new file.
 */
int zoidfs_create(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
                  int *created) 
{
   const char * path = zoidfs_resolve_path (parent_handle, component_name, 
         full_path); 
   int err; 
   int file;
   
   *created = 0; 

   /* first lookup the handle, creating one if needed  */ 
   persist_filename_to_handle (persist, path, handle, 1); 

   /* try to lookup the handle in the cache: maybe it is already open */
   if (handlecache_lookup (handle, &file))
   {
      /* handle was already in the cache: probably already openened/created */ 
      return ZFS_OK; 
   }

   /* OK: handle is not currently open, try to create */ 
   
   file = our_create (path, &err, created); 
   if (file < 0)
      return errno2zfs (err); 

   
   /* add to the open file cache */ 
   handlecache_add (handle, &file); 

   return ZFS_OK; 
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
                  zoidfs_cache_hint_t *to_parent_hint) 
{
   /* TODO: update path in database */ 
   return errno2zfs (-ENOSYS); 

   int ret = rename (from_full_path, to_full_path); 
   if (ret < 0)
      return errno2zfs(errno); 
   return ZFS_OK; 
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
                   const char *to_full_path, const zoidfs_sattr_t *sattr,
                   zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs_cache_hint_t *to_parent_hint) 
{
    return -ENOSYS;
}

static inline int posixcheck (int ret)
{
   if (ret < 0)
      return errno2zfs (errno); 
   else
      return ZFS_OK; 
}


/*
 * zoidfs_mkdir
 * This function creates a new directory.
 */
int zoidfs_mkdir(const zoidfs_handle_t *parent_handle,
                 const char *component_name, const char *full_path,
                 const zoidfs_sattr_t *sattr,
                 zoidfs_cache_hint_t *parent_hint) 
{
   const char * path = zoidfs_resolve_path (parent_handle, component_name, 
         full_path); 
   zoidfs_debug ("creating dir %s\n", path); 
   return posixcheck (mkdir (full_path, 0777) < 0); 
}


/*
 * zoidfs_readdir
 * This function returns the dirents from the specified parent directory. The
 * cookie is a pointer which specifies where in the directory to start
 * fetching the dirents from.
 */
int zoidfs_readdir(const zoidfs_handle_t *parent_handle,
                   zoidfs_dirent_cookie_t cookie, int *entry_count,
                   zoidfs_dirent_t *entries,
                   zoidfs_cache_hint_t *parent_hint) 
{

    return -ENOSYS;

}


/*
 * zoidfs_resize
 * This function resizes the file associated with the file handle.
 */
int zoidfs_resize(const zoidfs_handle_t *handle, uint64_t size) 
{
   //int file = handle_to_internal (handle); 
   

    return -ENOSYS;
}

static inline int saferead (int fd, void * buf, size_t count)
{
   /* handle signals and interruption: TODO */
   return read (fd, buf, count); 
}

static inline int safewrite (int fd, const void * buf, size_t count)
{
   return write (fd, buf, count); 
}

static inline int zoidfs_generic_access (const zoidfs_handle_t *handle, int mem_count,
                 void *mem_starts[], const size_t mem_sizes[],
                 int file_count, const uint64_t file_starts[],
                 uint64_t file_sizes[], int write) 
{
   /* note need lock here */
   int curmem = 0; 
   size_t memofs = 0 ; 
   int curfile = 0; 
   size_t fileofs = 0; 
   int file; 

   while (curmem < mem_count || curfile < file_count)
   {
      /* should always have same amount of bytes both in file and memory */

      /* determine largest amount of data to transfer */
      const size_t memremaining = mem_sizes[curmem] - memofs; 
      const size_t fileremaining = file_sizes[curfile] - fileofs; 

      const size_t thistransfer = mymin (memremaining, fileremaining); 

      char * mempos = ((char*) (mem_starts[curmem])) + memofs;
      uint64_t filepos = file_starts[curfile] + fileofs; 
      int ret; 
      
      assert (curmem < mem_count && curfile < file_count); 

      if (lseek64 (file, filepos, SEEK_SET) < 0)
         return errno2zfs (errno); 

      if (write)
         ret = safewrite (file, mempos, thistransfer); 
      else
         ret = saferead (file, mempos, thistransfer); 

      memofs += thistransfer;
      fileofs += thistransfer;
      if (memofs == mem_sizes[curmem])
      {
         ++curmem; memofs = 0; 
      }
      if (fileofs == file_sizes[curfile])
      {
         ++curfile; fileofs  = 0; 
      }
   }
   assert (curfile == file_count && curmem == mem_count); 
   return ZFS_OK; 
}


/*                  
 * zoidfs_write
 * This function implements the zoidfs write call.
 */
int zoidfs_write(const zoidfs_handle_t *handle, int mem_count,
                 const void *mem_starts[], const size_t mem_sizes[],
                 int file_count, const uint64_t file_starts[],
                 uint64_t file_sizes[]) 
{
    return zoidfs_generic_access (handle, mem_count, 
          (void ** ) mem_starts, mem_sizes, file_count, file_starts, file_sizes, 1);
}


/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
int zoidfs_read(const zoidfs_handle_t *handle, int mem_count,
                void *mem_starts[], const size_t mem_sizes[], int file_count,
                const uint64_t file_starts[], uint64_t file_sizes[]) 
{
    return zoidfs_generic_access (handle, mem_count, 
          mem_starts, mem_sizes, file_count, file_starts, file_sizes, 0);
}


/*
 * zoidfs_init
 * Initialize the client subsystems.
 */
int zoidfs_init(void) 
{
    return ZFS_OK;
}


/*
 * zoidfs_finalize
 * Finalize the client subsystems.
 */
int zoidfs_finalize(void) 
{
    return ZFS_OK;
}

