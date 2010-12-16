

#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

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

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/zoidfs-c-util.h"
#include "c-util/env-parse.h"
#include "c-util/tools.h"
#include "c-util/sha1.h"

#include "../zint-handler.h"
#include "fcache.h"   /* zoidfs handle <-> filename cache */
#include "dcache.h"   /* zoidfs handle <-> fd cache */

#include "c-util/configfile.h"

#ifndef NDEBUG
static int do_debug = 0;
#define zoidfs_debug(format, ...) if (do_debug) fprintf (stderr, "zoidfs_posix: debug: " format, ##__VA_ARGS__)
#else
#define zoidfs_debug(format, ...) {}
#endif

#define zoidfs_error(format, ...) fprintf (stderr, "zoidfs_posix: error: " format, ##__VA_ARGS__)

/* =============================== STATIC VARS ============================ */

static int posix_initialized                = 0;

static fcache_handle fcache;
static dcache_handle dcache;

/* ======================================================================== */

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
         zoidfs_debug ("%s", "invalid mode mask in posixmode_to_zoidfsattrtype!");
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
         zoidfs_error("%s", "Invalid file type in zoidfsattrtype_to_posixmode!\n");
         return 0;
   }
}



/* add two path components in a clean way
 * store in buf, at most bufsize
 */
static void path_add (const char * dir, const char * comp, char * buf, int bufsize)
{
   int dirlen;

   /* NOTE: does not handle . or .. */
   /* remove slash from comp if it starts with it */
   if (*comp == '/')
      ++comp;

#ifndef NDEBUG
   if (strlen(dir)+strlen(comp) >= (size_t) bufsize)
   {
      zoidfs_error ("%s", "Path too long in path_add (posix)!");
      exit (1);
   }
#endif

   dirlen = strlen(dir);
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


/**
 * Go over the path and remove extra / in the path
 */
static void zoidfs_path_fix_link (char * buf)
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

   /* terminate */
   *writeptr = 0;
}

/*
 * Like zoidfs_path_fix, but also remove / at the end.
 */
static void zoidfs_path_fix (char * buf)
{
   size_t len;
   zoidfs_path_fix_link (buf);

   len = strlen (buf);

   /*
    * This interferes with symlinks
    */

   /* if the last character is a /, remove it
    * unless it is also the first character */

   if (len <= 1)
      return;

   if (buf[len-1]=='/')
      buf[len-1] = 0;

}

/*
 * given a handle, return the filename for it.
 * returns 0 if the name is not in the cache
 */
static int handle2filename (const zoidfs_handle_t * handle,
      char * buf, int bufsize)
{
   assert(bufsize >= ZOIDFS_PATH_MAX);
   return filename_lookup (fcache, handle, buf, bufsize);
}

/*
 * Givern a handle, component, full_path tuple, try to construct a full posix
 * path
 *
 * Returns 1 if success,
 * 0 if failure (meaning ESTALE)
 */
static int zoidfs_simplify_path_mem (const zoidfs_handle_t * handle,
      const char * component, const char * full_path, char * buf, int bufsize)
{
   int ret;

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

   /*
    * Don't have full path, need to lookup the path associated with zoidfs
    * handle
    */
   ret = handle2filename (handle, buf, bufsize);

   /* Don't have the name: -> ESTALE */
   if (!ret)
      return 0;

   ret = strlen (buf);

#ifndef NDEBUG
   {
      int complen = strlen (component);

      /* take extra / into account */
      if (*component != '/')
         ++complen;

      assert (complen + ret < ZOIDFS_PATH_MAX);
   }
#endif


   /* add component to base path */
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

/*
 * buf should point to an array of at least ZOIDFS_PATH_MAX size
 * The funtion returns a pointer to the buffer to be used.
 * If possible, do not copy
 *
 * Return NULL if the path could not be constructed
 */
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
static int zoidfs_posix_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr, zoidfs_op_hint_t * UNUSED(op_hint))
{
   /* we could go for the fstat instead but that would open lots of files
    * without need */
   char buf[ZOIDFS_PATH_MAX];
   struct stat s;
   int ret;

   if (!handle2filename (handle, buf, sizeof(buf)))
      return ZFSERR_STALE;

   zoidfs_debug ("getattr %s\n",buf );

   ret = lstat (buf, &s);
   if (ret < 0)
      return errno2zfs (errno);

   if (attr->mask & (ZOIDFS_ATTR_FILEID|ZOIDFS_ATTR_FSID|ZOIDFS_ATTR_BSIZE))
   {
      zoidfs_debug ("%s", "zoidfs_posix: unable to return fileid,fsid or bsize!\n");
      attr->fsid=0;
      attr->blocksize=512;
      attr->fileid=0;
   }

   /* always retrieve everything */

   /* Don't modify the mask! */
   /* attr->mask = ZOIDFS_ATTR_MODE | ZOIDFS_ATTR_NLINK |
      ZOIDFS_ATTR_UID  |ZOIDFS_ATTR_GID | ZOIDFS_ATTR_SIZE |
      ZOIDFS_ATTR_ATIME | ZOIDFS_ATTR_CTIME | ZOIDFS_ATTR_MTIME; */

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
                   zoidfs_attr_t * rattr, zoidfs_op_hint_t * op_hint)
{
   char buf[ZOIDFS_PATH_MAX];

   if (!handle2filename (handle, buf, sizeof(buf)))
      return ZFSERR_STALE;


   if (sattr->mask & (ZOIDFS_ATTR_MODE))
   {
      if (chmod (buf, zoidfsmode_to_posix(sattr->mode))<0)
         return errno2zfs (errno);
   }

   if (sattr->mask & (ZOIDFS_ATTR_ATIME|ZOIDFS_ATTR_MTIME))
   {
      struct timeval tv[2];
      assert ((sattr->mask & (ZOIDFS_ATTR_ATIME|ZOIDFS_ATTR_MTIME))==
         (ZOIDFS_ATTR_ATIME|ZOIDFS_ATTR_MTIME));
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
      int ret = zoidfs_getattr (handle, rattr, op_hint);
      if (ret != ZFS_OK)
         return ret;
   }

   return ZFS_OK;
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 *
 * Buffer length is total buffer size.
 */
static int zoidfs_posix_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length, zoidfs_op_hint_t * UNUSED(op_hint))
{
   char buf[ZOIDFS_PATH_MAX];
   int ret;

   if (!handle2filename (handle, buf, sizeof(buf)))
      return ZFSERR_STALE;

   /* Buffer_length - 1 because we need to NULL-terminate */
   ret = readlink (buf, buffer, buffer_length - 1);
   if (ret < 0)
      return errno2zfs (errno);

   /* null-terminate */
   /* because the posix function acts like 'read' (returns number of bytes
    * read) while the zoidfs func uses the return code to indicate success */
   ALWAYS_ASSERT((size_t) ret < (size_t) buffer_length);

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


/*static int our_create (const char * filename, int * err, int * created,
      const zoidfs_sattr_t * sattr)
{
   int dummy;
   int fd;

   mode_t posixmode = S_IRUSR | S_IWUSR | S_IXUSR;

   if (sattr->mask & ZOIDFS_ATTR_MODE)
      posixmode = zoidfsmode_to_posix (sattr->mode);

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
         // File seems to exist already; Try to open
         fd = our_open (filename, err);
         if (fd < 0 && *err == ENOENT)
         {
            // strange: somebody removed in the mean time?
            zoidfs_debug ("File %s disappeared before we could open?? Retrying to create\n",
                  filename);
            continue;
         }
         // the file existed, so whatever reason our_open had for failing is
         // good enough for us
         return fd;
      }

      // failed, but not because the file already existed: give up
      return fd;
   }
}*/


/* Release obtained handle */
static int releasefd_handle (Descriptor * desc)
{
   return dcache_releasefd (dcache, desc);
}

/* Open a file descriptor for the specified zoidfs handle; File must exist
 * Return 1 on success, 0 on failure */
static int getfd_handle (const zoidfs_handle_t * handle, Descriptor * desc, int * err)
{
   int fd;
   char buf[ZOIDFS_PATH_MAX];

   do
   {

      /* first lookup in the cache */
      if (dcache_getfd(dcache, handle, desc))
      {
         /* we have handle in cache. Return that */
         return 1;
      }

      /* Handle is not there: try to map to a filename and reopen */
      if (!handle2filename (handle, buf, sizeof(buf)))
      {
         *err = ESTALE;
         return 0;   /* ESTALE */
      }

      /* we have the filename: open the file and add to the descriptor cache */
      fd = our_open (buf, err);
      if (fd < 0)
         return 0;

      /* Take an optimistic approach: if somebody else added the file at exactly
       * the same instance, close our fd and use the first one instead */
      if (!dcache_addfd (dcache, handle, fd, desc))
      {
         /* couldn't add the handle: may already exist */
         close (fd);
         if (desc->fd > 0)
         {
            /* FD is valid, somebody else openened the file before we could */
         }
         else
         {
            /* some other error: try again */
            continue;
         }
      }

      /* we got here, have obtained locked (in cache) file descriptor */

   } while (0);

   return 1;
}



/*
 * Generate a zoidfs handle for a given filename
 * The file should exist and filename should be full simplified/cleaned path
 *
 * We also use the inode number in an attempt to find out if a file was
 * removed and recreated with the same name.
 *
 */
int filename2handle (const struct stat * s, const char * buf, zoidfs_handle_t * h)
{
   unsigned int ofs = 0;
   uint32_t fixed_ino;

   SHA1Context con;
   SHA1Reset (&con);
   SHA1Input (&con, (const unsigned char *) buf, strlen(buf));
   SHA1Result (&con);

   memset (h->data, 0, sizeof(h->data));

   /* we have 5 bytes of SHA-1; skip reserved part */
   ofs += 4;  /* skip reserved part */

   memcpy (&h->data[ofs], &con.Message_Digest[0], sizeof (con.Message_Digest));
   ofs += sizeof (con.Message_Digest);

   fixed_ino = (uint32_t) s->st_ino;
   memcpy (&h->data[ofs], &fixed_ino, sizeof(fixed_ino));
   ofs += sizeof (fixed_ino);

   assert (ofs < sizeof (h->data));

   return 1;
}

/*
 * zoidfs_lookup
 * This function returns the file handle associated with the given file or
 * directory name.
 *
 * Note: zoidfs_lookup cannot remove a trailing '/' from the path /if the path
 * points to a symbolic link which points to a directory/!
 *
 * In other words, for a symbolic link a/b/c to a directory, a/b/c/ en a/b/c
 * have different zoidfs handles.
 */
static int zoidfs_posix_lookup(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_handle_t * handle, zoidfs_op_hint_t * UNUSED(op_hint))
{
   char newpath[ZOIDFS_PATH_MAX];
   struct stat s;

   if (!zoidfs_simplify_path_mem (parent_handle, component_name,
         full_path, newpath, sizeof(newpath)))
      return ZFSERR_STALE;

   zoidfs_debug ("lookup:  %s\n",newpath);

   /* needs to be lstat here: otherwise we fail to do a lookup on a broken
    * link... */
   if (lstat (newpath, &s) < 0)
      return errno2zfs (errno);

   filename2handle (&s, newpath, handle);

   /* add name to cache; replace if exists */
   filename_add (fcache, handle, newpath);

   return ZFS_OK;
}


/*
 * zoidfs_create
 * This function creates a new file.
 */
static int zoidfs_posix_create(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
                  int *created, zoidfs_op_hint_t * UNUSED(op_hint))
{
   char newpath[ZOIDFS_PATH_MAX];
   int file;
   struct stat s;
   int newmode;

   if (!zoidfs_simplify_path_mem (parent_handle, component_name,
         full_path,newpath,sizeof(newpath)))
      return ZFSERR_STALE;

   *created = 0;

   if (sattr->mask & ZOIDFS_ATTR_MODE)
      newmode = sattr->mode;
   else
      newmode = 0644;

   /* first see if the file exists */
   file = open (newpath, O_WRONLY|O_CREAT|O_EXCL, newmode);
   if (file >= 0)
   {
      *created = 1;
      close (file);
   }
   else
   {
      /* if the error is not that the file already exists, bail out */
      if (errno != EEXIST)
         return errno2zfs (errno);
   }

   /* file exists, and we should be able to stat it */
   if (lstat (newpath, &s) < 0)
      return errno2zfs (errno);

   filename2handle (&s, newpath, handle);

   /* add name to cache */
   filename_add (fcache, handle, newpath);

   return ZFS_OK;
}


/*
 * zoidfs_remove
 * This function removes the given file or directory.
 */
static int zoidfs_posix_remove(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_cache_hint_t * UNUSED(parent_hint), zoidfs_op_hint_t * UNUSED(op_hint))
{
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

   /* note we don't remove the entry from fcache/dcache */

   return ZFS_OK;
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
static int zoidfs_posix_commit(const zoidfs_handle_t * UNUSED(handle), zoidfs_op_hint_t * UNUSED(op_hint))
{
   /* file descriptor interface is not buffered... */
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
                  zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                  zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                  zoidfs_op_hint_t * UNUSED(op_hint))
{
   char p1[ZOIDFS_PATH_MAX];
   char p2[ZOIDFS_PATH_MAX];
   int ret;

   if (!zoidfs_simplify_path_mem (from_parent_handle,
         from_component_name, from_full_path, p1, sizeof(p1)))
      return ZFSERR_STALE;
   if (!zoidfs_simplify_path_mem (to_parent_handle,
         to_component_name, to_full_path, p2, sizeof(p2)))
      return ZFSERR_STALE;

   ret = rename (p1, p2);
   if (ret < 0)
      return errno2zfs (errno);

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
                   const char *to_full_path, const zoidfs_sattr_t * UNUSED(sattr),
                   zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                   zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                   zoidfs_op_hint_t * UNUSED(op_hint))
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
 *
 * NOTE: in the posix implementation, both filenames will have a different
 * handle
 */
static int zoidfs_posix_link(const zoidfs_handle_t * from_parent_handle /* in:ptr:nullok */,
                   const char * from_component_name /* in:str:nullok */,
                   const char * from_full_path /* in:str:nullok */,
                   const zoidfs_handle_t * to_parent_handle /* in:ptr:nullok */,
                   const char * to_component_name /* in:str:nullok */,
                   const char * to_full_path /* in:str:nullok */,
                   zoidfs_cache_hint_t * UNUSED(from_parent_hint) /* out:ptr:nullok */,
                   zoidfs_cache_hint_t * UNUSED(to_parent_hint) /* out:ptr:nullok */,
                   zoidfs_op_hint_t * UNUSED(op_hint))
{
   char p1[ZOIDFS_PATH_MAX];
   char p2[ZOIDFS_PATH_MAX];

   if (!zoidfs_simplify_path_mem (from_parent_handle,
         from_component_name, from_full_path, p1, sizeof(p1)))
      return ZFSERR_STALE;

   if (!zoidfs_simplify_path_mem (to_parent_handle,
         to_component_name, to_full_path, p2, sizeof(p2)))
      return ZFSERR_STALE;



   int ret = link (p1, p2);
   if (ret < 0)
      return errno2zfs (errno);

   return ZFS_OK;
}



/*
 * zoidfs_mkdir
 * This function creates a new directory.
 */
static int zoidfs_posix_mkdir(const zoidfs_handle_t *parent_handle,
                 const char *component_name, const char *full_path,
                 const zoidfs_sattr_t *sattr,
                 zoidfs_cache_hint_t * UNUSED(parent_hint),
                 zoidfs_op_hint_t * UNUSED(op_hint))
{
   char tmpbuf[ZOIDFS_PATH_MAX];
   int mode;
   const char * path = zoidfs_simplify_path (parent_handle, component_name,
         full_path, tmpbuf);
   if (!path)
      return ZFSERR_STALE;

   if (sattr->mask & ZOIDFS_ATTR_MODE)
      mode = sattr->mode;
   else
      mode = 0777;

   zoidfs_debug ("creating dir %s\n", path);
   return posixcheck (mkdir (path, mode));
}


/*
 * zoidfs_readdir
 * This function returns the dirents from the specified parent directory. The
 * cookie is a pointer which specifies where in the directory to start
 * fetching the dirents from.
 */
static int zoidfs_posix_readdir(const zoidfs_handle_t * handle,
                   zoidfs_dirent_cookie_t cookie, size_t *entry_count,
                   zoidfs_dirent_t *entries,
                   uint32_t flags,
                   zoidfs_cache_hint_t * UNUSED(parent_hint), zoidfs_op_hint_t * op_hint)
{
   char buf[ZOIDFS_PATH_MAX];
   char fullbuf[ZOIDFS_PATH_MAX];
   int returncode = 0;
   int i;

   /* if attributes are requested, enable the handle retrieval:
    * we need the handle anyway to get to the attributes */
   if (flags & ZOIDFS_ATTR_ALL)
      flags |= ZOIDFS_RETR_HANDLE;

   /* Try to lookup directory name */
   if (!filename_lookup (fcache, handle, buf, sizeof(buf)))
         return ZFSERR_STALE;

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
         //persist_clean_filename_to_handle (persist, fullbuf, &entries[ok].handle, 1);
      }

      if (flags & ZOIDFS_ATTR_ALL)
      {
         int ret = zoidfs_getattr (&entries[ok].handle, &entries[ok].attr, op_hint);
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
static int zoidfs_posix_resize(const zoidfs_handle_t *handle,
      zoidfs_file_size_t size, zoidfs_op_hint_t * UNUSED(op_hint))
{
   char buf[ZOIDFS_PATH_MAX];

   if (!filename_lookup (fcache, handle, buf, sizeof(buf)))
         return ZFSERR_STALE;

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
                 int file_count, const zoidfs_file_ofs_t file_starts[],
                 const zoidfs_file_size_t file_sizes[], int write,
                 zoidfs_op_hint_t * UNUSED(op_hint))
{
   int err;
   int file;
   int curmem = 0;
   size_t memofs = 0 ;
   int curfile = 0;
   size_t fileofs = 0;

   int ret;
   int zfs_ret = ZFS_OK;
   Descriptor desc;

   /* obtain file handle */
   ret = getfd_handle (handle, &desc, &err);
   if (!ret)
      return errno2zfs (err);

   file = desc.fd;

   /* note need lock here: why? */
   while (curmem < mem_count || curfile < file_count)
   {
      /* should always have same amount of bytes both in file and memory */

      /* determine largest amount of data to transfer */
      const size_t memremaining = mem_sizes[curmem] - memofs;
      const size_t fileremaining = file_sizes[curfile] - fileofs;

      const size_t thistransfer = zfsmin (memremaining, fileremaining);

      char * mempos = ((char*) (mem_starts[curmem])) + memofs;
      uint64_t filepos = file_starts[curfile] + fileofs;
      int ret;

      assert (curmem < mem_count && curfile < file_count);

      if (write)
         ret = safewrite (file, mempos, thistransfer, filepos);
      else
         ret = saferead (file, mempos, thistransfer, filepos);

      /* detect and set errors from IO calls */
      if(ret < 0 || (size_t)ret != thistransfer)
      {
        zfs_ret = ZFSERR_IO; 
      }

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

   /* release file desc */
   releasefd_handle (&desc);

   return zfs_ret;
}


/*
 * zoidfs_write
 * This function implements the zoidfs write call.
 */
static int zoidfs_posix_write(const zoidfs_handle_t *handle, size_t mem_count,
                 const void *mem_starts[], const size_t mem_sizes[],
                 size_t file_count, const zoidfs_file_ofs_t file_starts[],
                 const zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint)
{
    return zoidfs_generic_access (handle, mem_count,
          (void ** ) mem_starts, mem_sizes, file_count, file_starts, file_sizes, 1, op_hint);
}


/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
static int zoidfs_posix_read(const zoidfs_handle_t *handle, size_t mem_count,
                void *mem_starts[], const size_t mem_sizes[], size_t file_count,
                const zoidfs_file_ofs_t file_starts[],
                const zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint)
{
    return zoidfs_generic_access (handle, mem_count,
          mem_starts, mem_sizes, file_count, file_starts, file_sizes, 0, op_hint);
}


/*
 * zoidfs_init
 * Initialize the client subsystems.
 */
static int zoidfs_posix_init(void)
{
   static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
   int ret = ZFS_OK;

   pthread_mutex_lock (&mutex);

   if (!posix_initialized)
   {
      do
      {
         int namesize = 4096;
         int fdsize = 128;

#ifndef NDEBUG
         if (env_parse_have_debug ("posix"))
            do_debug=1;
#endif

         if (getenv ("ZOIDFS_HANDLECACHE_SIZE"))
            fdsize = atoi (getenv("ZOIDFS_HANDLECACHE_SIZE"));
         if (getenv ("ZOIDFS_NAMECACHE_SIZE"))
            namesize = atoi (getenv("ZOIDFS_NAMECACHE_SIZE"));

         /* create name cache and fd cache */
         fcache = filename_create (namesize);
         dcache = dcache_create (fdsize);

         zoidfs_debug ("%s", "zoidfs_init called...\n");
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

   zoidfs_debug ("%s", "zoidfs_finalize called...\n");

   pthread_mutex_lock (&mutex);
   do
   {
      if (!posix_initialized)
         break;

      filename_destroy (fcache);
      dcache_destroy (dcache);

      posix_initialized = 0;

   } while (0);

   pthread_mutex_unlock (&mutex);
   return ret;
}

static int zoidfs_posix_resolve_path (const char * local_path,
      char * fs_path, int fs_path_max, zoidfs_handle_t * UNUSED(newhandle),
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

static int zoidfs_posix_set_options(ConfigHandle UNUSED(c), SectionHandle UNUSED(s))
{
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
   zoidfs_posix_finalize,
   zoidfs_posix_set_options
};


