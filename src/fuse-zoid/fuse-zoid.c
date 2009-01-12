#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>

#include "zoidfs.h"

#define ZFUSE_DIRENTRY_COUNT 128
#define ZFUSE_INITIAL_HANDLES 16

#define ZFUSE_XATTR_ZOIDFSHANDLE "zoidfs_handle"

/* the following is C99 safe */ 
#ifndef NDEBUG
#define zfuse_debug(format, ...) fprintf (stderr, format,##__VA_ARGS__)
#else
#define zfuse_debug(format, ...) {}
#endif


/* ======================================================================= */
/* ======================================================================= */
/* ======================================================================= */

typedef struct 
{
   zoidfs_handle_t handle; 
} zfuse_handle_t; 


static zfuse_handle_t * zfuse_handles  = NULL; 
static int            zfuse_handle_capacity = 0; 
static int            zfuse_handle_free = 0; 
static int            zfuse_handle_used = 0; 

static int zfuse_handle_add (const zoidfs_handle_t * handle)
{
   int newpos; 
   int i; 

   assert (sizeof(zfuse_handle_t) >= sizeof(int));

   /* check for first use */ 
   if (!zfuse_handles)
   {
      zfuse_debug ("Initializing zfuse_handles\n"); 
      int i; 

      zfuse_handles = calloc (ZFUSE_INITIAL_HANDLES, sizeof (zfuse_handle_t));
      zfuse_handle_used = 0; 
      zfuse_handle_capacity = ZFUSE_INITIAL_HANDLES; 
      zfuse_handle_free = 0; 

      assert (zfuse_handle_capacity); 

      /* init free list */
      for (i=0; i<zfuse_handle_capacity; ++i)
      {
         *((int *) &zfuse_handles[i]) = 
            (i == zfuse_handle_capacity - 1 ? -1 : i+1); 
      }
   }

   /* check for full */ 
   if (zfuse_handle_used == zfuse_handle_capacity)
   {
      /* extend */ 
      int oldcap = zfuse_handle_capacity; 

      assert (zfuse_handle_free == -1); 

      zfuse_handle_capacity *= 2; 
      zfuse_handles =
          realloc (zfuse_handles, sizeof(zfuse_handle_t)*zfuse_handle_capacity);

      for (i=oldcap; i<zfuse_handle_capacity; ++i)
      {
         *((int *) &zfuse_handles[i]) = 
            (i == zfuse_handle_capacity - 1 ? -1 : i+1); 
      }

      zfuse_handle_free = oldcap; 
   }

   assert (zfuse_handle_used < zfuse_handle_capacity); 
   assert (zfuse_handle_free != -1 && zfuse_handle_free < zfuse_handle_capacity); 

   newpos = zfuse_handle_free; 

   zfuse_handle_free = *((int*) &zfuse_handles[zfuse_handle_free]); 
   ++zfuse_handle_used; 
   zfuse_handles[newpos].handle = *handle; 
   return newpos; 
}

static void zfuse_handle_remove (int pos)
{
   assert (pos >= 0 && pos < zfuse_handle_capacity); 
   assert (zfuse_handle_used); 

   *((int *) &zfuse_handles[pos]) = zfuse_handle_free; 
   zfuse_handle_free = pos; 
   --zfuse_handle_used; 
}

static const zoidfs_handle_t * zfuse_handle_lookup (int pos)
{
   assert (pos >= 0 && pos < zfuse_handle_capacity); 
   return &zfuse_handles[pos].handle; 
}



/* ======================================================================= */
/* ======================================================================= */
/* ======================================================================= */

inline static void posixmode_to_sattr (mode_t mode, zoidfs_sattr_t * a)
{
   a->mode = mode;
}

inline static void posixtime_to_zoidfs (const time_t * t1, zoidfs_time_t * t2)
{
   t2->seconds = *t1; 
   t2->useconds = 0; 
}

inline static void zoidfstime_to_posix (const zoidfs_time_t * t1, time_t * t2)
{
   /* ignore microseconds from zoidfs */
   *t2 = t1->seconds; 
}


inline static void zoidfstime_current (zoidfs_time_t * t)
{
   time_t p = time (NULL); 
   posixtime_to_zoidfs (&p, t); 
}

static int zoidfserr_to_posix (int ret)
{
   /* note not complete */ 
   switch (ret)
   {
      case ZFS_OK:
         return 0; 
      case ZFSERR_PERM:
         return -EPERM;
      case ZFSERR_NOENT:
         return -ENOENT;
      case ZFSERR_IO:
         return -EIO; 
      case ZFSERR_NXIO:
         return -ENXIO; 
      case ZFSERR_ACCES:
         return -EACCES;
      case ZFSERR_EXIST:
         return -EEXIST;
      case ZFSERR_NOTDIR:
         return -ENOTDIR;
      case ZFSERR_ISDIR:
         return -EISDIR; 
   }
   return -ENOSYS; 
}

static int zfuse_getattr(const char * path, 
      struct stat * stbuf)
{
   int res = 0; 

   zfuse_debug ("zfuse_getattr: %s\n", path); 

   memset(stbuf, 0, sizeof(struct stat));
   if(strcmp(path, "/") == 0) 
   {
      stbuf->st_mode = S_IFDIR | 0755;
      stbuf->st_nlink = 2;
   }
   else
   {
      zoidfs_handle_t handle; 
      zoidfs_attr_t attr; 
      int ret = zoidfs_lookup (NULL, NULL, path, &handle);
      if (ret)
         return zoidfserr_to_posix (ret); 
      ret = zoidfs_getattr (&handle, &attr); 

      if (ret)
         return zoidfserr_to_posix (ret); 

      stbuf->st_mode = attr.mode; 
      stbuf->st_nlink = attr.nlink; 
      stbuf->st_uid = attr.uid; 
      stbuf->st_gid = attr.gid; 
      stbuf->st_size = attr.size;
      stbuf->st_blocks = attr.blocksize; 
      zoidfstime_to_posix (&attr.atime, &stbuf->st_atime);
      zoidfstime_to_posix (&attr.ctime, &stbuf->st_ctime);
      zoidfstime_to_posix (&attr.mtime, &stbuf->st_mtime);
   }
   return res;
}

static int zfuse_truncate (const char * path, off_t size)
{
   zoidfs_handle_t handle; 
   int ret = zoidfs_lookup (NULL, NULL, path, &handle);
   if (ret)
      return zoidfserr_to_posix (ret); 
   return zoidfserr_to_posix (zoidfs_resize (&handle, size)); 
}

static int zfuse_mkdir (const char * path, mode_t mode)
{
   zoidfs_sattr_t zattr; 
   
   zattr.mask = ZOIDFS_ATTR_MODE; 
   posixmode_to_sattr (mode, &zattr); 

   zfuse_debug ("zfuse_mkdir %s\n", path); 

   return zoidfserr_to_posix (zoidfs_mkdir (NULL, NULL, path, &zattr,
            NULL));
}

static int zfuse_rmdir (const char * path)
{
   /* no rmdir in ZOIDFS? */ 
   zfuse_debug ("zfuse_rmdir %s\n", path); 
   return zoidfserr_to_posix (zoidfs_remove (NULL, NULL, path, NULL)); 
}

static int zfuse_opendir(const char * path, struct fuse_file_info * fi)
{
   int ret; 
   zoidfs_handle_t dirhandle; 

   zfuse_debug ("zfuse_opendir: fi->fh = %lu\n", fi->fh); 
   ret =zoidfs_lookup(NULL, NULL, path, &dirhandle);
   if (ret)
   {
      zfuse_debug("zfuse_readdir: lookup of %s returned error: %i\n", 
            path, ret); 
      return zoidfserr_to_posix (ret); 
   }

   fi->fh = zfuse_handle_add (&dirhandle); 

   return 0; 
}

static int zfuse_releasedir (const char * path, struct fuse_file_info * fi)
{
   zfuse_debug ("zfuse_releasedir: path=%s, fh=%lu\n", path, 
         (unsigned long) fi->fh); 
   zfuse_handle_remove (fi->fh); 
   return 0; 
}

static int zfuse_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
   /* probably could use fuse_file_info handle here */
   const zoidfs_handle_t * dirhandle; 
   int ret; 
   zoidfs_dirent_t * entries; 
   int entrycount; 
   zoidfs_dirent_cookie_t cookie = 0;  
   
   dirhandle = zfuse_handle_lookup (fi->fh); 
   
   filler(buf, ".", NULL, 0);
   filler(buf, "..", NULL, 0);

   entries = malloc (ZFUSE_DIRENTRY_COUNT * sizeof(zoidfs_dirent_t)); 
   do
   {
      int i; 
      entrycount = ZFUSE_DIRENTRY_COUNT; 
      ret = zoidfs_readdir (dirhandle, cookie, &entrycount, entries, 
            NULL); 
      if (ret)
      {
         zfuse_debug ("zoidfs_readdir returned error: %i\n", ret); 
         return zoidfserr_to_posix (ret); 
      }

      for (i=0; i<entrycount; ++i)
      {
         zfuse_debug ("zoidfs_readdir: %s\n", entries[i].name); 
         filler (buf, entries[i].name, NULL, 0 ); 
      }

      /* If we got less than expected, assume end of dir */ 
      if (entrycount < ZFUSE_DIRENTRY_COUNT)
         break; 

      /* set cookie for next round */
      assert(entrycount); 
      cookie = entries[entrycount-1].cookie; 
   } while (1); 

    return 0;
}

static int zfuse_open(const char *path, struct fuse_file_info *fi)
{
   int ret; 
   zoidfs_handle_t handle; 

   /* fuse_file_info.flags contains open flags */
   /* fi.direct_io can be set by us to enable directio */

   zfuse_debug ("zfuse_open: %s\n", path); 

   ret = zoidfs_lookup (NULL, NULL, path, &handle); 
   if (ret != ZFS_OK)
   {
      zfuse_debug ("zoidfs_lookup of %s returned error: %i\n", path, ret); 
      return zoidfserr_to_posix (ret); 
   }

   fi->direct_io = 0; 
   fi->keep_cache = 0; 
   
   fi->fh = zfuse_handle_add (&handle); 


    return 0;
}

static int zfuse_write (const char * path, const char * buf, size_t size, 
      off_t offset, struct fuse_file_info * fi)
{
   const zoidfs_handle_t * handle = zfuse_handle_lookup (fi->fh); 
   zfuse_debug ("Writing to handle %lu\n", (unsigned long) fi->fh); 
   return size; 
}

static int zfuse_read(const char *path, char *buf, size_t size, off_t offset,
                      struct fuse_file_info *fi)
{
   const zoidfs_handle_t * handle = zfuse_handle_lookup (fi->fh); 
   zfuse_debug ("Reading from handle %lu\n", (unsigned long) fi->fh); 
   return -ENOSYS;
}

static void * zfuse_init (struct fuse_conn_info *conn)
{
   zfuse_debug("ZFuse Init\n");
   return 0; 
}


static void zfuse_destroy (void * d)
{
   zfuse_debug ("ZFuse debug\n"); 
}

static int zfuse_unlink (const char * path)
{
   zfuse_debug ("zfuse_unlink %s\n", path); 
   return zoidfserr_to_posix (zoidfs_remove (NULL, NULL, 
            path, NULL)); 
}

static int zfuse_release (const char * path, struct fuse_file_info * fi)
{
   zfuse_debug ("zfuse_release %s, fh: %lu\n", path, (unsigned long) fi->fh); 
   zfuse_handle_remove (fi->fh); 
   return 0;
}


static int zfuse_listxattr (const char * file, char * data, size_t size)
{
   const int needed = strlen (ZFUSE_XATTR_ZOIDFSHANDLE)+1; 

   zfuse_debug ("zfuse_listxattr: %s, datasize=%i\n", file, size); 

   if (!size)
      return needed; 

   if (size < needed)
      return -ERANGE;

   strcpy (data, ZFUSE_XATTR_ZOIDFSHANDLE); 
   return needed;
}

static int zfuse_getxattr (const char * path, const char * name,
      char * value, size_t size)
{
   const int needed = sizeof (zoidfs_handle_t); 
   int ret; 

   if (!size)
      return needed;

   if (size < needed)
      return -ERANGE; 

   ret = zoidfs_lookup (NULL, NULL, path, (zoidfs_handle_t*) value); 
   if (ret != ZFS_OK)
   {
      zfuse_debug ("zfuse_getxattr: lookup failed on %s\n", path); 
      return zoidfserr_to_posix (ret); 
   }
   
   return sizeof (zoidfs_handle_t); 
}

static struct fuse_operations hello_oper = {
    .getattr	= zfuse_getattr,
    .readdir	= zfuse_readdir,
    .open	= zfuse_open,
    .read	= zfuse_read,
    .rmdir      = zfuse_rmdir,
    .mkdir      = zfuse_mkdir,
    .init       = zfuse_init,
    .destroy    = zfuse_destroy,
    .unlink     = zfuse_unlink,
    .release    = zfuse_release,
    .getxattr   = zfuse_getxattr,
    .listxattr  = zfuse_listxattr,
    .opendir    = zfuse_opendir, 
    .releasedir = zfuse_releasedir
};

int main(int argc, char *argv[])
{
    return fuse_main(argc, argv, &hello_oper, NULL);
}


