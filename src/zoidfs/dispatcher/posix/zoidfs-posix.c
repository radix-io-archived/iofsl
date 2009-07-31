

#define _LARGEFILE64_SOURCE

/* for pread/pwrite */
#define _XOPEN_SOURCE 500

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
#include <dirent.h>
#include <sys/time.h>
#include <unistd.h>

#include <pthread.h>

#include "zoidfs.h"
#include "handlecache.h"
#include "persist.h"
#include "zoidfs-util.h"
#include "env-parse.h"
#include "persist-clean.h"

#include "../zint-handler.h"

#define mymin(a,b) ((a)<(b) ? (a):(b))

#ifndef NDEBUG
static int do_debug = 0; 
#define zoidfs_debug(format, ...) if (do_debug) fprintf (stderr, "zoidfs_posix: debug: " format, ##__VA_ARGS__)
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

static int posix_initialized                = 0;
static persist_op_t  *  persist         = 0; 



static inline void zoidfs_handle_fix (zoidfs_handle_t * h)
{
   ZINT_MARK_HANDLE(h,0); 
}


static inline zoidfs_attr_type_t posixmode_to_zoidfsattrtype (mode_t mode)
{
   switch (mode & S_IFMT)
   {
      case S_IFSOCK:
         return ZOIDFS_SOCK;
      case S_IFLNK:
         return ZOIDFS_LNK;
      case S_IFREG:
         return ZOIDFS_REG;
      case S_IFBLK:
         return ZOIDFS_BLK;
      case S_IFDIR:
         return ZOIDFS_DIR; 
      case S_IFCHR:
         return ZOIDFS_CHR; 
      case S_IFIFO:
         return ZOIDFS_FIFO;
      default:
         zoidfs_debug ("invalid mode mask in posixmode_to_zoidfsattrtype!"); 
         return ZOIDFS_INVAL; 
   }
}

static inline mode_t zoidfsattrtype_to_posixmode (zoidfs_attr_type_t t)
{
   switch (t)
   {
      case ZOIDFS_REG:
         return S_IFREG;
      case ZOIDFS_DIR:
         return S_IFDIR; 
      case ZOIDFS_LNK:
         return S_IFLNK; 
      case ZOIDFS_CHR:
         return S_IFCHR; 
      case ZOIDFS_BLK:
         return S_IFBLK;
      case ZOIDFS_FIFO:
         return S_IFIFO; 
      case ZOIDFS_SOCK:
         return S_IFSOCK;
      case ZOIDFS_INVAL:
      default:
         zoidfs_error("Invalid file type in zoidfsattrtype_to_posixmode!\n"); 
         return 0; 
   }
}

static void path_add (const char * dir, const char * comp, char * buf, int bufsize)
{
   /* NOTE: does not handle . or .. */ 
   /* remove slash from comp if it starts with it */ 
   if (*comp == '/')
      ++comp; 

   int dirlen = strlen(dir); 
   assert (dirlen); 
   if (dir[dirlen-1]=='/')
      snprintf (buf, bufsize, "%s%s", dir, comp); 
   else
      snprintf (buf, bufsize, "%s/%s", dir, comp); 
}

static inline int errno2zfs (int val)
{
   if (val == 0)
      return ZFS_OK; 
   switch(val)
   {
      case EPERM:
         return ZFSERR_PERM;
      case ENOENT:
         return ZFSERR_NOENT;
      case EIO: 
         return ZFSERR_IO;
      case ENXIO: 
         return ZFSERR_NXIO;
      case EACCES:
         return ZFSERR_ACCES;
      case EEXIST:
         return ZFSERR_EXIST;
      case ENODEV:
         return ZFSERR_NODEV;
      case ENOTDIR:
         return ZFSERR_NOTDIR;
      case EISDIR: 
         return ZFSERR_ISDIR;
      case EFBIG:
         return ZFSERR_FBIG;
      case ENOSPC:
         return ZFSERR_NOSPC;
      case EROFS:
         return ZFSERR_ROFS;
      case ENAMETOOLONG:
         return ZFSERR_NAMETOOLONG;
      case ENOTEMPTY:
         return ZFSERR_NOTEMPTY;
      case EDQUOT:
         return ZFSERR_DQUOT;
      case ESTALE:
         return ZFSERR_STALE;
      default:
         return ZFSERR_OTHER; 
   }
}

static inline int posixcheck (int ret)
{
   if (ret < 0)
      return errno2zfs (errno); 
   else
      return ZFS_OK; 
}


inline static void posixmode_to_zoidfs (const mode_t * m, uint16_t * mode)
{
   /* only return true file mode bits: zoidfs keeps file type bits in
    * attr.type */ 
   *mode = *m & (S_ISUID| S_ISGID| S_ISVTX| S_IRWXU| S_IRUSR| S_IWUSR|\
         S_IXUSR| S_IRWXG| S_IRGRP| S_IWGRP| S_IXGRP| S_IRWXO| S_IROTH| S_IWOTH| S_IXOTH); 
}

inline static mode_t zoidfsmode_to_posix (uint16_t mode)
{
   /* zoidfs mode is a subset of the posix mode so we just
    * return the value */ 
   return mode; 
}


/* convert posix time to zoidfs time */ 
inline static void posixtime_to_zoidfs (const time_t * t1, zoidfs_time_t * t2)
{
   t2->seconds = *t1; 
   t2->nseconds = 0; 
}

inline static void zoidfstime_to_posix (const zoidfs_time_t * t1, time_t * t2)
{
   /* ignore microseconds from zoidfs */
   *t2 = t1->seconds; 
}


void zoidfstime_to_timeval (const zoidfs_time_t * t1, struct timeval * t2)
{
   t2->tv_sec = t1->seconds;
   t2->tv_usec = t1->nseconds / 1000; 
}

static void zoidfs_path_fix (char * buf)
{
   const char * readptr = buf; 
   char * writeptr = buf;
   int last = 0; 
   char c; 

   while ((c=*readptr++))
   {
      /* if the last char was a '/' then skip it if this one is also '/' */
      if (c=='/' && last)
         continue; 
      last = c == '/';
      if (readptr != writeptr)
         *writeptr = c; 

      writeptr++; 
   }

   /* if the last character is a /, remove it
    * unless it is also the first character */
   if (last && (writeptr - buf) != 1)
      --writeptr; 

   /* terminate */ 
   *writeptr = 0; 
}


static int zoidfs_simplify_path_mem (const zoidfs_handle_t * handle,
      const char * component, const char * full_path, char * buf, int bufsize)
{
   if (full_path)
   {
#ifndef NDEBUG
      int len = strlen (full_path); 
      assert (len+1 < bufsize); 
#endif
      strncpy(buf, full_path, bufsize);
      buf[bufsize-1]=0; 
      zoidfs_path_fix (buf); 
      return 1; 
   }

   int ret = persist_clean_handle_to_filename (persist, handle, buf, bufsize); 
   if (!ret)
   {
#ifdef USE_ESTALE
      zoidfs_debug ("zoidfs_simplify_path stale handle!\n"); 
      return 0; 
#else
      zoidfs_error ("zoidfs_simplify_path: unable to lookup handle->path!\n"); 
      exit (1);
#endif
   }

#ifndef NDEBUG
   {
      const int complen = strlen (component); 
      assert (complen + ret < ZOIDFS_PATH_MAX); 
   }
#endif
   if (*component != '/')
   {
      buf[ret]='/';
      ++ret; 
   }
   strcpy (&buf[ret], component); 
   zoidfs_path_fix (buf); 
   return 1; 
}

/* Check for '//' */
static inline int zoidfs_path_ok (const char * p)
{
   return strstr (p, "//") == NULL; 
}

/* buf should point to an array of at least ZOIDFS_PATH_MAX size */
static inline const char * zoidfs_simplify_path (const zoidfs_handle_t * handle, 
      const char * component, const char * full_path, char * buf)
{
   if (full_path && zoidfs_path_ok(full_path))
      return full_path; 

   /* if zoidfs_simplify_path_mem failed it must be ESTALE */
   if (! zoidfs_simplify_path_mem (handle, component, full_path, buf, ZOIDFS_PATH_MAX))
      return 0; 

   return buf;
}


/*
 * zoidfs_null
 * This function implements a noop operation. The IOD returns a 1-byte message
 * to the CN.
 */
static int zoidfs_posix_null(void) 
{
   return ZFS_OK; 
}


/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle from
 * the ION.
 */
static int zoidfs_posix_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr) 
{
   /* we could go for the fstat instead but that would open lots of files
    * without need */ 
   char buf[ZOIDFS_PATH_MAX]; 

   if (!persist_clean_handle_to_filename (persist, handle, buf, sizeof(buf)))
   {
      zoidfs_error ("stale handle in getattr: %s\n", zoidfs_handle_string(handle)); 
       return -ZFSERR_STALE; 
   }
   zoidfs_debug ("getattr %s\n",buf ); 

   struct stat s; 
   int ret = lstat (buf, &s); 
   if (ret < 0)
      return errno2zfs (errno); 

   if (attr->mask & (ZOIDFS_ATTR_FILEID|ZOIDFS_ATTR_FSID|ZOIDFS_ATTR_BSIZE))
   {
      zoidfs_debug ("zoidfs_posix: unable to return fileid,fsid or bsize!\n"); 
      attr->fsid=0; 
      attr->blocksize=512; 
      attr->fileid=0; 
   }

   /* always retrieve everything */ 
   attr->mask = ZOIDFS_ATTR_MODE | ZOIDFS_ATTR_NLINK | 
      ZOIDFS_ATTR_UID  |ZOIDFS_ATTR_GID | ZOIDFS_ATTR_SIZE |
      ZOIDFS_ATTR_ATIME | ZOIDFS_ATTR_CTIME | ZOIDFS_ATTR_MTIME;

   posixtime_to_zoidfs (&s.st_ctime,  &attr->ctime); 
   posixtime_to_zoidfs (&s.st_atime,  &attr->atime); 
   posixtime_to_zoidfs (&s.st_mtime,  &attr->mtime); 
   posixmode_to_zoidfs (&s.st_mode, &attr->mode); 
   attr->type = posixmode_to_zoidfsattrtype (s.st_mode); 
   attr->nlink = s.st_nlink;
   attr->uid = s.st_uid; 
   attr->gid = s.st_gid; 
   attr->size = s.st_size; 
    return ZFS_OK;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
static int zoidfs_posix_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t * rattr) 
{
   char buf[ZOIDFS_PATH_MAX];
   if (!persist_clean_handle_to_filename (persist, handle, buf, sizeof (buf)))
   {
      zoidfs_error ("stale handle in getattr: %s\n", zoidfs_handle_string(handle)); 
       return -ZFSERR_STALE; 
   }

   if (sattr->mask & (ZOIDFS_ATTR_MODE))
   {
      if (chmod (buf, zoidfsmode_to_posix(sattr->mode))<0)
         return errno2zfs (errno); 
   }

   if (sattr->mask & (ZOIDFS_ATTR_ATIME|ZOIDFS_ATTR_MTIME))
   {
      assert ((sattr->mask & (ZOIDFS_ATTR_ATIME|ZOIDFS_ATTR_MTIME))==
         (ZOIDFS_ATTR_ATIME|ZOIDFS_ATTR_MTIME));
      struct timeval tv[2]; 
      zoidfstime_to_timeval (&sattr->atime, &tv[0]); 
      zoidfstime_to_timeval (&sattr->mtime, &tv[1]); 
      if (utimes (buf, &tv[0])<0)
         return errno2zfs (errno); 
   }

   if (sattr->mask & (ZOIDFS_ATTR_UID|ZOIDFS_ATTR_GID))
   {
      gid_t gid = -1;
      uid_t uid = -1; 
      if (sattr->mask & ZOIDFS_ATTR_UID)
         uid = sattr->uid; 
      if (sattr->mask & ZOIDFS_ATTR_GID)
         gid = sattr->gid; 
      if (chown (buf, uid, gid)<0)
         return errno2zfs (errno); 
   }

   if (rattr->mask & ZOIDFS_ATTR_ALL)
   {
      int ret = zoidfs_getattr (handle, rattr); 
      if (ret != ZFS_OK)
         return ret; 
   }

   return ZFS_OK; 
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 */
static int zoidfs_posix_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length) 
{
   char buf[ZOIDFS_PATH_MAX];

   if (!persist_clean_handle_to_filename (persist, handle, buf, sizeof(buf)))
   {
#ifdef USE_ESTALE
      zoidfs_debug ("Returned -ESTALE\n"); 
      return -ESTALE; 
#else
      zoidfs_error ("Unable to map zoidfs handle to path! Invalid handle?\n"); 
      exit (1); 
#endif
   }
   int ret = readlink (buf, buffer, buffer_length); 
   if (ret < 0)
      return errno2zfs (errno); 

   /* null-terminate */ 
   /* because the posix function acts like 'read' (returns number of bytes
    * read) while the zoidfs func uses the return code to indicate success */ 
   buffer[ret] = 0; 
   return ZFS_OK; 
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


static int our_create (const char * filename, int * err, int * created,
      const zoidfs_sattr_t * sattr)
{
   int dummy; 
   int fd; 

   mode_t posixmode = S_IRUSR | S_IWUSR | S_IXUSR; 

   if (sattr->mask & ZOIDFS_ATTR_MODE)
      posixmode = zoidfsmode_to_posix (sattr->mode); 

   /* allow NULL for created */ 
   if (!created)
      created = &dummy; 


   while (1)
   {

      fd = open (filename, O_CREAT|O_EXCL|O_RDWR, posixmode); 

      if (fd >= 0)
      {
         *created = 1; 
         return fd; 
      }

      *err = errno; 

      if (*err == EEXIST)
      {
         /* File seems to exist already; Try to open */ 
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
static int getfd_handle (const zoidfs_handle_t * handle, int * err)
{
   int fd; 
   char buf[ZOIDFS_PATH_MAX]; 

   /* first lookup in the cache */
   if (handlecache_lookup (handle, &fd))
      return fd; 

   /* Handle is not there: try to map to a filename and reopen */ 
   if (!persist_clean_handle_to_filename (persist, handle, buf, sizeof(buf)))
   {
#ifdef USE_ESTALE
      zoidfs_debug ("Returned -ESTALE\n"); 
      return -ESTALE; 
#else
      zoidfs_error ("Unable to map zoidfs handle to path! Invalid handle?\n"); 
      exit (1); 
#endif
   }

   fd = our_open (buf, err); 
   if (fd < 0)
      return -1;

   /* add entry to cache */ 
   handlecache_add (handle, &fd); 

   return fd; 
}

/*
 * zoidfs_lookup
 * This function returns the file handle associated with the given file or
 * directory name.
 */
static int zoidfs_posix_lookup(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_handle_t *handle) 
{
   char newpath[ZOIDFS_PATH_MAX]; 
   if (!zoidfs_simplify_path_mem (parent_handle, component_name, 
         full_path, newpath, sizeof(newpath)))
      return ZFSERR_STALE; 

   zoidfs_debug ("lookup:  %s\n",newpath); 

   struct stat s; 

   /* needs to be lstat here: otherwise we fail to do a lookup on a broken
    * link... */ 
   if (lstat (newpath, &s) < 0)
      return errno2zfs (errno); 

   /* Lookup the persist handle for the file, creating if needed */ 
   persist_clean_filename_to_handle (persist, newpath, handle, 1); 

   return ZFS_OK; 
 
   // ignore component_name
   int err; 
   int file = our_open (newpath, &err); 

   if (file < 0)
   {
      /* it could be that we fail because we don't have read permission. (i.e.
       * directory)
       * That does not mean we should not return a valid handle for it.
       *
       * Try stat and if that works
       */


      return errno2zfs (err); 
   }

   /* Lookup the persist handle for the file, creating if needed */ 
   persist_clean_filename_to_handle (persist, newpath, handle, 1); 

   /* Instead of keeping the file open, we close it again and
    * wait and see if somebody actually reads from it */ 
   close (file); 

   /* file openened ok, add to cache */
   /* handlecache_add (handle, &file); */

   return ZFS_OK; 
}


/*
 * zoidfs_remove
 * This function removes the given file or directory.
 */
static int zoidfs_posix_remove(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_cache_hint_t *parent_hint) 
{
   zoidfs_handle_t handle; 
   char tmpbuf[ZOIDFS_PATH_MAX]; 
   const char * path = zoidfs_simplify_path (parent_handle, component_name, 
         full_path, tmpbuf); 

   if (!path)
      return ZFSERR_STALE;

   // ignore component_name
   if (unlink (path)< 0) 
   {
      int ret = errno; 
      if (ret != EISDIR)
         return errno2zfs (ret); 

      /* is directory */
      if (rmdir (path)<0)
         return errno2zfs (errno); 
   }
   
   /* TODO: probably there is a race condition here */

   /* Remove from handlecache, otherwise create might return a handle to the
    * old file instead of to the new one 
    * NOTE: this might cause other processes to fail I/O */
   if (persist_clean_filename_to_handle (persist, path, &handle, 0))
   {
      /* If we cannot lookup the handle anymore we can no longer flush the entry
       * from the handlecache. However, this is not a problem since 
       * create/lookup will also fail to retrieve the handle and generate a
       * new one. 
       */
      handlecache_purge (&handle);       
   
      /* remove from database */
      persist_clean_purge (persist, path, 0); 
   } 
   

   return ZFS_OK; 
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
static int zoidfs_posix_commit(const zoidfs_handle_t *handle) 
{
   /* file descriptor interface is not buffered... */ 
   return ZFS_OK; 
}


/*
 * zoidfs_create
 * This function creates a new file.
 */
static int zoidfs_posix_create(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
                  int *created) 
{
   char newpath[ZOIDFS_PATH_MAX];
   int err; 
   int file;

   if (!zoidfs_simplify_path_mem (parent_handle, component_name, 
         full_path,newpath,sizeof(newpath))) 
      return ZFSERR_STALE;
   
   *created = 0; 

   /* first lookup the handle, creating one if needed  */ 
   persist_clean_filename_to_handle (persist, newpath, handle, 1); 

   /* try to lookup the handle in the cache: maybe it is already open */
   if (handlecache_lookup (handle, &file))
   {
      /* handle was already in the cache: probably already openened/created */ 
      return ZFS_OK; 
   }

   /* OK: handle is not currently open, try to create */ 
   
   file = our_create (newpath, &err, created, sattr); 

   /* TODO: do sattr stuff */ 

   if (file < 0)
      return errno2zfs (err); 

   
   /* add to the open file cache */ 
   handlecache_add (handle, &file); 

   return ZFS_OK; 
}


/*
 * zoidfs_rename
 * This function renames an existing file or directory.
 *
 * NOTE: this contains a race condition since the file and the database cannot
 * be updated at the same time
 */
static int zoidfs_posix_rename(const zoidfs_handle_t *from_parent_handle,
                  const char *from_component_name,
                  const char *from_full_path,
                  const zoidfs_handle_t *to_parent_handle,
                  const char *to_component_name,
                  const char *to_full_path,
                  zoidfs_cache_hint_t *from_parent_hint,
                  zoidfs_cache_hint_t *to_parent_hint) 
{
   char p1[ZOIDFS_PATH_MAX]; 
   char p2[ZOIDFS_PATH_MAX]; 

   if (!zoidfs_simplify_path_mem (from_parent_handle, 
         from_component_name, from_full_path, p1, sizeof(p1)))
      return ZFSERR_STALE; 
   if (!zoidfs_simplify_path_mem (to_parent_handle,
         to_component_name, to_full_path, p2, sizeof(p2)))
      return ZFSERR_STALE; 
  
   int ret = rename (p1, p2); 
   if (ret < 0)
      return errno2zfs (errno); 

   /* TODO: update path in database */ 


   return ZFS_OK; 
}


/*
 * zoidfs_symlink
 * This function creates a symbolic link.
 */
static int zoidfs_posix_symlink(const zoidfs_handle_t *from_parent_handle,
                   const char *from_component_name,
                   const char *from_full_path,
                   const zoidfs_handle_t *to_parent_handle,
                   const char *to_component_name,
                   const char *to_full_path, const zoidfs_sattr_t *sattr,
                   zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs_cache_hint_t *to_parent_hint) 
{
   char p1[ZOIDFS_PATH_MAX]; 
   char p2[ZOIDFS_PATH_MAX]; 

   if (!zoidfs_simplify_path_mem (from_parent_handle, 
         from_component_name, from_full_path, p1, sizeof(p1)))
      return ZFSERR_STALE; 

   if (!zoidfs_simplify_path_mem (to_parent_handle,
         to_component_name, to_full_path, p2, sizeof(p2)))
      return ZFSERR_STALE; 
   
   return posixcheck (symlink(p1, p2));
}


/**
 * Create hard link 
 */
static int zoidfs_posix_link(const zoidfs_handle_t * from_parent_handle /* in:ptr:nullok */,
                   const char * from_component_name /* in:str:nullok */,
                   const char * from_full_path /* in:str:nullok */,
                   const zoidfs_handle_t * to_parent_handle /* in:ptr:nullok */,
                   const char * to_component_name /* in:str:nullok */,
                   const char * to_full_path /* in:str:nullok */,
                   zoidfs_cache_hint_t * from_parent_hint /* out:ptr:nullok */,
                   zoidfs_cache_hint_t * to_parent_hint /* out:ptr:nullok */)
{
   char p1[ZOIDFS_PATH_MAX]; 
   char p2[ZOIDFS_PATH_MAX]; 

   if (!zoidfs_simplify_path_mem (from_parent_handle, 
         from_component_name, from_full_path, p1, sizeof(p1)))
      return ZFSERR_STALE; 

   if (!zoidfs_simplify_path_mem (to_parent_handle,
         to_component_name, to_full_path, p2, sizeof(p2)))
      return ZFSERR_STALE; 

   zoidfs_handle_t oldhandle; 
   

   int ret = link (p1, p2); 
   if (ret < 0)
      return errno2zfs (errno); 

   if (ret >= 0)
   {
      /* Need to ensure that both filenames have the same zoidfs handle */
      persist_clean_filename_to_handle (persist, p1, &oldhandle, 1); 
      if (!persist_clean_add (persist, p2, &oldhandle))
      {
         zoidfs_error ("Unable to add handle for %s file in hardlinking %s to %s??\n",
               p2, p1, p2); 

         /* assume it failed because entry existed. Try to fix */ 
         persist_clean_purge (persist, p2, 0); 
         persist_clean_add (persist, p2, &oldhandle); 
      }
   }

   return ZFS_OK; 
}



/*
 * zoidfs_mkdir
 * This function creates a new directory.
 */
static int zoidfs_posix_mkdir(const zoidfs_handle_t *parent_handle,
                 const char *component_name, const char *full_path,
                 const zoidfs_sattr_t *sattr,
                 zoidfs_cache_hint_t *parent_hint) 
{
   char tmpbuf[ZOIDFS_PATH_MAX]; 
   const char * path = zoidfs_simplify_path (parent_handle, component_name, 
         full_path, tmpbuf); 
   if (!path)
      return ZFSERR_STALE;

   zoidfs_debug ("creating dir %s\n", path); 
   return posixcheck (mkdir (path, 0777)); 
}


/*
 * zoidfs_readdir
 * This function returns the dirents from the specified parent directory. The
 * cookie is a pointer which specifies where in the directory to start
 * fetching the dirents from.
 */
static int zoidfs_posix_readdir(const zoidfs_handle_t *parent_handle,
                   zoidfs_dirent_cookie_t cookie, size_t *entry_count,
                   zoidfs_dirent_t *entries,
                   uint32_t flags, 
                   zoidfs_cache_hint_t *parent_hint) 
{
   char buf[ZOIDFS_PATH_MAX]; 
   char fullbuf[ZOIDFS_PATH_MAX]; 
   int returncode = 0; 
   int i; 

   /* if attributes are requested, enable the handle retrieval:
    * we need the handle anyway to get to the attributes */ 
   if (flags & ZOIDFS_ATTR_ALL)
      flags |= ZOIDFS_RETR_HANDLE; 

   if (!persist_clean_handle_to_filename (persist, parent_handle, buf, sizeof(buf)))
      return -ZFSERR_STALE; 

   assert (sizeof(zoidfs_dirent_cookie_t) >= sizeof(off_t)); 

   zoidfs_debug ("readdir: %s\n", buf); 

   DIR * d = opendir (buf); 
   if (!d)
      return errno2zfs (errno); 

   if (cookie)
      seekdir (d, (off_t) cookie); 

   size_t len = offsetof(struct dirent, d_name) +
                     pathconf(buf, _PC_NAME_MAX) + 1; 
   struct dirent * e = malloc(len);

   const int bufsize =*entry_count;
   int ok = 0; 
   for (i=0; i<bufsize; ++i)
   {
      struct dirent * res =0; 
      int ret = readdir_r (d, e, &res);
      if (ret < 0)
      {
         zoidfs_debug ("readdir_r failed\n"); 
         returncode = errno; 
         break; 
      }

      if (!res)
         break; 

      strncpy (entries[ok].name, res->d_name, sizeof(entries[ok].name)); 

      if (flags & ZOIDFS_RETR_HANDLE)
      {
         /* we need to retrieve handles for the dir entries */ 
         path_add (buf, entries[ok].name, fullbuf, sizeof(fullbuf)); 
         persist_clean_filename_to_handle (persist, fullbuf, &entries[ok].handle, 1);
      }

      if (flags & ZOIDFS_ATTR_ALL)
      {
         int ret = zoidfs_getattr (&entries[ok].handle, &entries[ok].attr);
         if (ret != ZFS_OK)
         {
            free (e); 
            closedir (d); 
            return errno2zfs(ret); 
         }
      }
      entries[ok].cookie = telldir (d); 

      ++ok; 
   }

   *entry_count = ok; 
   closedir (d); 
   free (e); 
   return errno2zfs(returncode); 
}


/*
 * zoidfs_resize
 * This function resizes the file associated with the file handle.
 */
static int zoidfs_posix_resize(const zoidfs_handle_t *handle, uint64_t size) 
{
   char buf[ZOIDFS_PATH_MAX];
   if (!persist_clean_handle_to_filename (persist, handle, buf, sizeof(buf)))
   {
#ifdef USE_ESTALE
      zoidfs_debug ("zoidfs_simplify_path unknown handle!\n"); 
      return 0; 
#else
      zoidfs_error ("zoidfs_simplify_path: unable to lookup handle->path!\n"); 
      exit (1);
#endif
   }

   /* we avoid ftruncate since that would cause the file to be openend */ 
   return posixcheck (truncate (buf, size)); 
}

static inline int saferead (int fd, void * buf, size_t count, uint64_t filepos)
{
   /* handle signals and interruption: TODO */
   return pread (fd, buf, count, filepos); 
}

static inline int safewrite (int fd, const void * buf, size_t count,
      uint64_t filepos)
{
   return pwrite (fd, buf, count, filepos); 
}

static inline int zoidfs_generic_access (const zoidfs_handle_t *handle, int mem_count,
                 void *mem_starts[], const size_t mem_sizes[],
                 int file_count, const uint64_t file_starts[],
                 uint64_t file_sizes[], int write) 
{
   int err; 
   int file; 
   
   /* obtain file handle */ 
   file = getfd_handle (handle, &err); 
   if (file < 0)
      return errno2zfs (err); 
  

   /* note need lock here */
   int curmem = 0; 
   size_t memofs = 0 ; 
   int curfile = 0; 
   size_t fileofs = 0; 

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

      if (write)
         ret = safewrite (file, mempos, thistransfer, filepos); 
      else
         ret = saferead (file, mempos, thistransfer, filepos); 

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
static int zoidfs_posix_write(const zoidfs_handle_t *handle, size_t mem_count,
                 const void *mem_starts[], const size_t mem_sizes[],
                 size_t file_count, const uint64_t file_starts[],
                 uint64_t file_sizes[]) 
{
    return zoidfs_generic_access (handle, mem_count, 
          (void ** ) mem_starts, mem_sizes, file_count, file_starts, file_sizes, 1);
}


/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
static int zoidfs_posix_read(const zoidfs_handle_t *handle, size_t mem_count,
                void *mem_starts[], const size_t mem_sizes[], size_t file_count,
                const uint64_t file_starts[], uint64_t file_sizes[]) 
{
    return zoidfs_generic_access (handle, mem_count, 
          mem_starts, mem_sizes, file_count, file_starts, file_sizes, 0);
}


static int zoidfs_hc_removefunc (hc_item_value_t * value)
{
   zoidfs_debug ("zoidfs_hc_removefunc: closing fd %i\n", *(int*)value); 
   /* value is pointer to file descriptor that is being removed */
   close (*(int*)value); 
   return 1; 
}

/*
 * zoidfs_init
 * Initialize the client subsystems.
 */
static int zoidfs_posix_init(void) 
{
   static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
   const char * dbstr;
   int ret = ZFS_OK; 

   pthread_mutex_lock (&mutex); 

   if (!posix_initialized)
   {
      do
      {
#ifndef NDEBUG
         if (env_parse_have_debug ("posix"))
            do_debug=1; 
#endif
         dbstr  = getenv ("ZOIDFS_PERSIST"); 

         persist = 0; 
         if (!dbstr)
         {
            zoidfs_error ("ZOIDFS_PERSIST environment variable not set!\n"); 
            ret = ZFSERR_OTHER; 
            break; 
         }
         persist = persist_init (dbstr); 
         if (!persist)
         {
            zoidfs_error ("Error initializing persistance layer (%s)\n", 
                  dbstr); 
            ret= ZFSERR_OTHER; 
            break; 
         }

         int size = 128;
         if (getenv ("ZOIDFS_HANDLECACHE_SIZE"))
            size = atoi (getenv("ZOIDFS_HANDLECACHE_SIZE")); 
         handlecache_init (size, zoidfs_hc_removefunc); 

         zoidfs_debug ("zoidfs_init called...\n"); 
         posix_initialized = 1; 

      } while (0);
   }

   pthread_mutex_unlock (&mutex); 

   return ret;
}


/*
 * zoidfs_finalize
 * Finalize the client subsystems.
 */
static int zoidfs_posix_finalize(void) 
{
   static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 
   int ret = ZFS_OK; 
      
   zoidfs_debug ("zoidfs_finalize called...\n"); 

   pthread_mutex_lock (&mutex); 
   do
   {
      if (!posix_initialized)
         break; 

      handlecache_destroy (); 

      assert (persist); 
      persist_done (persist); 

      persist=0; 
      posix_initialized = 0; 

   } while (0); 

   pthread_mutex_unlock (&mutex); 
   return ret; 
}

static int zoidfs_posix_resolve_path (const char * local_path,
      char * fs_path, int fs_path_max, zoidfs_handle_t * newhandle,
      int * usenew)
{
   *usenew = 0; 
   

   if (fs_path)
   {
      strncpy (fs_path, local_path, fs_path_max); 
      fs_path[fs_path_max-1]=0; 
   }
   return ZFS_OK; 
}

zint_handler_t posix_handler = {
   zoidfs_posix_null,
   zoidfs_posix_getattr,
   zoidfs_posix_setattr,
   zoidfs_posix_lookup,
   zoidfs_posix_readlink,
   zoidfs_posix_read,
   zoidfs_posix_write,
   zoidfs_posix_commit,
   zoidfs_posix_create,
   zoidfs_posix_remove,
   zoidfs_posix_rename,
   zoidfs_posix_link,
   zoidfs_posix_symlink,
   zoidfs_posix_mkdir,
   zoidfs_posix_readdir,
   zoidfs_posix_resize,
   zoidfs_posix_resolve_path,
   zoidfs_posix_init,
   zoidfs_posix_finalize
};


