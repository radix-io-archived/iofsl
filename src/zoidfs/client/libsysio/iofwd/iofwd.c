/*
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Jason Cope 
 * Argonne National Laboratory
 * copej@mcs.anl.gov
 *
 * This driver is based on the native driver implemntation of
 *  libsysio.
 */

#include <errno.h>
#include <stdarg.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <dirent.h>

#include "sysio.h"
#include "xtio.h"
#include "fs.h"
#include "inode.h"
#include "mount.h"
#include "dev.h"


#include "iofwd.h"

#include <string.h>

#include "zoidfs.h"

/*
 * Debug and trace facilities
 */

//#define DEBUG_SYSIO_IOFWD

#ifdef DEBUG_SYSIO_IOFWD
#define SYSIO_IOFWD_FENTER()                                                            \
do{                                                                                     \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i ENTER\n", __func__, __FILE__, __LINE__);   \
}while(0)

#define SYSIO_IOFWD_FEXIT()                                                             \
do{                                                                                     \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i EXIT\n", __func__, __FILE__, __LINE__);    \
}while(0)

#define SYSIO_IOFWD_FABORT()                                                            \
do{                                                                                     \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i ABORT\n", __func__, __FILE__, __LINE__);   \
}while(0)

#else
#define SYSIO_IOFWD_FENTER()    /* not in debug mode... do nothing */
#define SYSIO_IOFWD_FEXIT()     /* not in debug mode... do nothing */
#define SYSIO_IOFWD_FABORT()    /* not in debug mode... do nothing */
#endif

#define SYSIO_IOFWD_FINFO(...)                                                                          \
do{                                                                                                     \
    char __buffer[4096];                                                                                \
    sprintf(__buffer, ##__VA_ARGS__);                                                                   \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i INFO, %s\n", __func__, __FILE__, __LINE__, __buffer);      \
}while(0)

int _iofwd_sysio_init();
int _iofwd_sysio_drv_init_all();
//void __attribute__((constructor)) _iofwd_sysio_startup(void); /* Force the startup of the sysio iofwd driver before main() is invoked */
void _iofwd_sysio_startup(void); /* Force the startup of the sysio iofwd driver before main() is invoked */

static int iofwd_fsswop_mount(const char *source, unsigned flags, const void *data, struct pnode *tocover, struct mount **mntp);
struct fssw_ops iofwd_fssw_ops = {iofwd_fsswop_mount};

static int create_internal_namespace(const void *data);

static void iofwd_fsop_gone(struct filesys *fs);
static struct filesys_ops iofwd_inodesys_ops = {iofwd_fsop_gone,};

static struct inode * iofwd_i_new(struct filesys *fs, time_t expiration, struct intnl_stat *buf);
static struct inode * iofwd_iget(struct filesys *fs, time_t expire, struct intnl_stat *stbp);
static int iofwd_ibind(struct filesys *fs, char *path, time_t t, struct inode **inop);
static int iofwd_i_invalid(struct inode *inop, struct intnl_stat *stat);

/*
 * IOFWD operations mapped to the driver
 */
static int iofwd_inop_lookup(struct pnode *pno, struct inode **inop, struct intent *intnt, const char *path);
static int iofwd_inop_getattr(struct pnode *pno, struct intnl_stat *stbuf);
static int iofwd_inop_setattr(struct pnode *pno, unsigned mask, struct intnl_stat *stbuf);
static ssize_t iofwd_filldirentries(struct pnode *pno, _SYSIO_OFF_T *posp, char *buf, size_t nbytes);
static int iofwd_inop_mkdir(struct pnode *pno, mode_t mode);
static int iofwd_inop_rmdir(struct pnode *pno);
static int iofwd_inop_symlink(struct pnode *pno, const char *data);
static int iofwd_inop_readlink(struct pnode *pno, char *buf, size_t bufsiz);
static int iofwd_inop_open(struct pnode *pno, int flags, mode_t mode);
static int iofwd_inop_close(struct pnode *pno);
static int iofwd_inop_link(struct pnode *old, struct pnode *new);
static int iofwd_inop_unlink(struct pnode *pno);
static int iofwd_inop_rename(struct pnode *old, struct pnode *new);
static int iofwd_inop_read(struct ioctx *ioctx);
static int iofwd_inop_write(struct ioctx *ioctx);
static _SYSIO_OFF_T iofwd_inop_pos(struct pnode *pno, _SYSIO_OFF_T off);
static int iofwd_inop_iodone(struct ioctx *ioctx);
static int iofwd_inop_fcntl(struct pnode *pno, int cmd, va_list ap, int *rtn);
static int iofwd_inop_sync(struct pnode *pno);
static int iofwd_inop_datasync(struct pnode *pno);
static int iofwd_inop_ioctl(struct pnode *pno, unsigned long int request, va_list ap);
static int iofwd_inop_mknod(struct pnode *pno, mode_t mode, dev_t dev);
#ifdef _HAVE_STATVFS
static int iofwd_inop_statvfs(struct pnode *pno, struct intnl_statvfs *buf);
#endif
static void iofwd_inop_gone(struct inode *ino);

/*
 * inode operations for zoidfs
 */
static struct inode_ops iofwd_i_ops = {
    inop_lookup: iofwd_inop_lookup,
    inop_getattr: iofwd_inop_getattr,
    inop_setattr: iofwd_inop_setattr,
    inop_filldirentries: iofwd_filldirentries,
    inop_filldirentries2: NULL,
    inop_mkdir: iofwd_inop_mkdir,
    inop_rmdir: iofwd_inop_rmdir,
    inop_symlink: iofwd_inop_symlink,
    inop_readlink: iofwd_inop_readlink,
    inop_open: iofwd_inop_open,
    inop_close: iofwd_inop_close,
    inop_link: iofwd_inop_link,
    inop_unlink: iofwd_inop_unlink,
    inop_rename: iofwd_inop_rename,
    inop_read: iofwd_inop_read,
    inop_write: iofwd_inop_write,
    inop_pos: iofwd_inop_pos,
    inop_iodone: iofwd_inop_iodone,
    inop_fcntl: iofwd_inop_fcntl,
    inop_sync: iofwd_inop_sync,
    inop_datasync: iofwd_inop_datasync,
    inop_ioctl: iofwd_inop_ioctl,
    inop_mknod: iofwd_inop_mknod,
#ifdef _HAVE_STATVFS
    inop_statvfs: iofwd_inop_statvfs,
#endif
    inop_perms_check: _sysio_p_generic_perms_check,
    inop_gone: iofwd_inop_gone
};

/*
 *
 * Define weak symbols for all the I/O operations that we will 
 *  capture and forward to zoidfs. These weak symbols are replaced
 *  by libsysio. If we don't capture these, the iofwd sysio
 *  constructor will not execute in static libs since it
 *  can't detect an iofwd op was invoked.
 *
 * These should never execute! If they execute, the appropriate
 *  operation was not implemented...
 *
 */
int __attribute__((weak)) SYSIO_INTERFACE_NAME(open)(const char * path, int oflag, ...)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FINFO("WARNING: not implemented!");
    SYSIO_IOFWD_FEXIT();
    return 0;
}

int __attribute__((weak)) SYSIO_INTERFACE_NAME(close)(int oflag)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FINFO("WARNING: not implemented!");
    SYSIO_IOFWD_FEXIT();
    return 0;
}

int __attribute__((weak)) SYSIO_INTERFACE_NAME(mkdir)(const char * path, mode_t oflag)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FINFO("WARNING: not implemented!");
    SYSIO_IOFWD_FEXIT();
    return 0;
}

/*int __attribute__((weak)) SYSIO_INTERFACE_NAME(rmdir)(const char * path)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FINFO("WARNING: not implemented!");
    SYSIO_IOFWD_FEXIT();
    return 0;
}*/

/*int __attribute__((weak)) SYSIO_INTERFACE_NAME(unlink)(const char * path)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FINFO("WARNING: not implemented!");
    SYSIO_IOFWD_FEXIT();
    return 0;
}*/

static inline int zfs_error_to_sysio_error(int val)
{
   if (val == 0)
      return 0;
   switch(val)
   {
      case ZFSERR_PERM:
         return EPERM;
      case ZFSERR_NOENT:
         return ENOENT;
      case ZFSERR_IO:
         return EIO;
      case ZFSERR_NXIO:
         return ENXIO;
      case ZFSERR_ACCES:
         return EACCES;
      case ZFSERR_EXIST:
         return EEXIST;
      case ZFSERR_NODEV:
         return ENODEV;
      case ZFSERR_NOTDIR:
         return ENOTDIR;
      case ZFSERR_ISDIR:
         return EISDIR;
      case ZFSERR_FBIG:
         return EFBIG;
      case ZFSERR_NOSPC:
         return ENOSPC;
      case ZFSERR_ROFS:
         return EROFS;
      case ZFSERR_NAMETOOLONG:
         return ENAMETOOLONG;
      case ZFSERR_NOTEMPTY:
         return ENOTEMPTY;
      case ZFSERR_DQUOT:
         return EDQUOT;
      case ZFSERR_STALE:
         return ESTALE;
      default:
         return EINVAL;
   }
}

/*
 * Internal library data structures
 */
static struct mount * iofwd_internal_mount = NULL;

struct iofwd_filesystem {
    time_t  iofwdfs_atimo;              /* attr timeout (sec) */
};

/*
 * Given fs, return driver private part.
 */
#define FS2IOFWDFS(fs) \
    ((struct iofwd_filesystem *)(fs)->fs_private)

#define I2II(ino)   ((struct iofwd_inode *)((ino)->i_private))

/*
 * iofwd file identifiers format.
 */
struct iofwd_inode_identifier {
    dev_t   dev;                    /* device number */
    ino_t   ino;                    /* i-number */
#ifdef HAVE_GENERATION
    unsigned int gen;               /* generation number */
#endif
};

/*
 * Driver-private i-node information
 */
struct iofwd_inode {
    unsigned
        ii_seekok       : 1,                    /* can seek? */
        ii_attrvalid        : 1,                /* cached attrs ok? */
        ii_resetfpos        : 1,                /* reset fpos? */
        ii_handlevalid      : 1;                /* is the handle valid? */
    struct iofwd_inode_identifier ii_ident;     /* unique identifier */
    struct file_identifier ii_fileid;           /* ditto */
    int ii_fd;                                  /* host fildes */
    int ii_oflags;                              /* flags, from open */
    unsigned ii_nopens;                         /* soft ref count */
    _SYSIO_OFF_T ii_fpos;                       /* current pos */
    time_t  ii_attrtim;                         /* attrs expire time */
    time_t  ii_handletim;                       /* handle expire time */
    zoidfs_handle_t handle;                     /* zoidfs handle */
};

void zoidfs_print_handle(struct iofwd_inode *iino)
{
    uint8_t v[32];
    unsigned char hbuffer[65];
    int i = 0;
    unsigned char * buffer = NULL;
    unsigned char * vptr = NULL;
    zoidfs_handle_t * h = NULL;

    if(!iino)
        return;

    h = &(iino->handle);
    memset(hbuffer, 0, 65);
    memset(v, 0, 32);

    memcpy(&v[0], &(h->data[0]), 32);
    buffer = (unsigned char *)v;
    vptr = hbuffer;

    while(i < sizeof(zoidfs_handle_t))
    {
        int j = 0;
        unsigned char cb = buffer[i];
        while(j < 2)
        {
            unsigned int val = (cb >> 4) & 0xf;
            if(val < 10)
            {
                *vptr++ = (unsigned char)('0' + val);
            }
            else
            {
                *vptr++ = (unsigned char)('a' + (val - 10));
            }
            cb = (cb << 4);
            j += 1;
        }
        i += 1;
    }
    fprintf(stderr, "%s : handle = %s\n", __func__, hbuffer);
}

#define SYSIO_COPY_STAT(src, dest) *(dest) = *(src) 

#define SYSIO_PRINT_STAT(src) \
do { \
    fprintf(stderr, "dev = %u\n", (src)->st_dev);\
    fprintf(stderr, "ino = %u\n", (src)->st_ino);\
    fprintf(stderr, "mode = %o\n", (src)->st_mode & 0777);\
    if(S_ISDIR((src)->st_mode)) {fprintf(stderr, "is dir\n");}\
    else {fprintf(stderr, "is not dir\n"); } \
    fprintf(stderr, "nlink = %u\n", (src)->st_nlink);\
    fprintf(stderr, "uid = %u\n", (src)->st_uid);\
    fprintf(stderr, "gid = %u\n", (src)->st_gid);\
    fprintf(stderr, "rdev = %u\n", (src)->st_rdev);\
    fprintf(stderr, "size = %i\n", (src)->st_size);\
    fprintf(stderr, "blksize = %u\n", (src)->st_blksize);\
    fprintf(stderr, "blocks = %i\n", (src)->st_blocks);\
    fprintf(stderr, "atime = %lu\n", (src)->st_atime);\
    fprintf(stderr, "mtime = %lu\n", (src)->st_mtime);\
    fprintf(stderr, "ctime = %lu\n", (src)->st_ctime);\
} while(0);


#if defined(_LARGEFILE64_SOURCE) && defined(SYS_lstat64)
#define SYSIO_SYS_stat      SYS_lstat64
#elif defined(SYS_lstat)
#define SYSIO_SYS_stat      SYS_lstat
#endif

#if defined(_LARGEFILE64_SOURCE) && defined(SYS_fstat64)
#define SYSIO_SYS_fstat     SYS_fstat64
#elif defined(SYS_fstat)
#define SYSIO_SYS_fstat     SYS_fstat
#endif

#define COPY_SYSIO_ZFS_ATTR(src, dest)              \
do {                                                \
    (dest)->mask = ZOIDFS_ATTR_ALL;                 \
    (dest)->mode = (src)->st_mode & 0777;           \
    if(S_ISREG((src)->st_mode))                     \
    {                                               \
        (dest)->type = ZOIDFS_REG;                  \
    else if(S_ISDIR((src)->st_mode))                \
    {                                               \
        (dest)->type = ZOIDFS_DIR;                  \
    }                                               \
    else if(S_ISCHR((src)->st_mode))                \
    {                                               \
        (dest)->type = ZOIDFS_CHR;                  \
    }                                               \
    else if(S_ISBLK((src)->st_mode))                \
    {                                               \
        (dest)->type = ZOIDFS_BLK;                  \
    }                                               \
    else if(S_ISFIFO((src)->st_mode))               \
    {                                               \
        (dest)->type = ZOIDFS_FIFO;                 \
    }                                               \
    else if(S_ISLNK((src)->st_mode))                \
    {                                               \
        (dest)->type = ZOIDFS_LNK;                  \
    }                                               \
    else if(S_ISSOCK((src)->st_mode))               \
    {                                               \
        (dest)->type = ZOIDFS_SOCK;                 \
    }                                               \
    else                                            \
    {                                               \
        (dest)->type = ZOIDFS_INVAL;                \
    }                                               \
    (dest)->uid = (src)->st_uid;                    \
    (dest)->gid = (src)->st_gid;                    \
    (dest)->size = (src)->st_size;                  \
    (dest)->blocksize = (src)->st_blksize;          \
    (dest)->nlink = (src)->st_nlink;                \
    (dest)->atime.seconds = (src)->st_atime;        \
    (dest)->atime.nseconds = 0;                     \
    (dest)->mtime.seconds = (src)->st_mtime;        \
    (dest)->mtime.nseconds = 0;                     \
    (dest)->ctime.seconds = (src)->st_ctime;        \
    (dest)->ctime.nseconds = 0;                     \
    (dest)->fileid = (src)->st_ino;                 \
    (dest)->fsid = (src)->st_dev;                   \
} while(0);

#define COPY_ZFS_SYSIO_ATTR(src, dest)              \
do {                                                \
    (dest)->st_mode = (src)->mode & 0777;           \
    if((src)->type == ZOIDFS_REG)                   \
    {                                               \
       (dest)->st_mode = (dest)->st_mode | S_IFREG; \
    }                                               \
    else if((src)->type == ZOIDFS_DIR)              \
    {                                               \
       (dest)->st_mode = (dest)->st_mode | S_IFDIR; \
    }                                               \
    else if((src)->type == ZOIDFS_LNK)              \
    {                                               \
       (dest)->st_mode = (dest)->st_mode | S_IFLNK; \
    }                                               \
    else if((src)->type == ZOIDFS_CHR)              \
    {                                               \
       (dest)->st_mode = (dest)->st_mode | S_IFCHR; \
    }                                               \
    else if((src)->type == ZOIDFS_BLK)              \
    {                                               \
       (dest)->st_mode = (dest)->st_mode | S_IFBLK; \
    }                                               \
    else if((src)->type == ZOIDFS_FIFO)             \
    {                                               \
       (dest)->st_mode = (dest)->st_mode | S_IFIFO; \
    }                                               \
    (dest)->st_uid = (src)->uid;                    \
    (dest)->st_gid = (src)->gid;                    \
    (dest)->st_size = (src)->size;                  \
    (dest)->st_blksize = (src)->blocksize;          \
    (dest)->st_nlink = (src)->nlink;                \
    (dest)->st_atime = (src)->atime.seconds;        \
    (dest)->st_mtime = (src)->mtime.seconds;        \
    (dest)->st_ctime = (src)->ctime.seconds;        \
    (dest)->st_ino = (src)->fileid;                 \
    (dest)->st_dev = (src)->fsid;                   \
} while(0);

#define COPY_ZFS_SYSIO_SATTR(src, dest)             \
do {                                                \
    (dest)->st_mode = (src)->mode & 0777;           \
    (dest)->st_uid = (src)->uid;                    \
    (dest)->st_gid = (src)->gid;                    \
    (dest)->st_size = (src)->size;                  \
    (dest)->st_atime = (src)->atime.seconds;        \
    (dest)->st_mtime = (src)->mtime.seconds;        \
} while(0);

#define COPY_SYSIO_ZFS_SATTR(src, dest)             \
do {                                                \
    (dest)->mask = ZOIDFS_ATTR_ALL;                 \
    (dest)->mode = (src)->st_mode & 0777;           \
    (dest)->uid = (src)->st_uid;                    \
    (dest)->gid = (src)->st_gid;                    \
    (dest)->size = (src)->st_size;                  \
    (dest)->atime.seconds = (src)->st_atime;        \
    (dest)->atime.nseconds = 0;                     \
    (dest)->mtime.seconds = (src)->st_mtime;        \
    (dest)->mtime.nseconds = 0;                     \
} while(0);

static inline int iofwd_attrs_valid(struct iofwd_inode * iino, time_t t)
{
    return (iino->ii_attrtim && (t < iino->ii_attrtim));
}

static inline int iofwd_handle_valid(struct iofwd_inode * iino, time_t t)
{
    if(iino)
    {
        //return (iino->ii_handlevalid && iino->ii_handletim && (t < iino->ii_handletim));
        return (iino->ii_handletim && (t < iino->ii_handletim));
    }
    return 0;
}

/*
 * stat the file remotely
 */
static int iofwd_stat(const char *path, struct inode *ino, time_t t, struct intnl_stat *buf)
{
    SYSIO_IOFWD_FENTER();
    struct iofwd_inode *iino;
    int err;
    struct intnl_stat stbuf;
    zoidfs_handle_t fhandle;
    unsigned fhandle_set = 0;
    zoidfs_attr_t attr;


    iino = ino ? I2II(ino) : NULL;

    /* if the inode is valid, don't lookup */
    if(iino && iofwd_handle_valid(iino, t))
    {
        attr.mask = ZOIDFS_ATTR_ALL | (ZOIDFS_ATTR_FILEID|ZOIDFS_ATTR_FSID|ZOIDFS_ATTR_BSIZE);
        err = zoidfs_getattr(&iino->handle, &attr, ZOIDFS_NO_OP_HINT);
        if(err == ZFS_OK)
        {
            COPY_ZFS_SYSIO_ATTR(&attr, &stbuf);
        }
    }
    /* if the path is set */
    else if (path)
    {
        err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
        if(err == ZFS_OK)
        {
            fhandle_set = 1;

            /* get the attrs for the file handle */
            attr.mask = ZOIDFS_ATTR_ALL | (ZOIDFS_ATTR_FILEID|ZOIDFS_ATTR_FSID|ZOIDFS_ATTR_BSIZE);
            err = zoidfs_getattr(&fhandle, &attr, ZOIDFS_NO_OP_HINT);
            if(err == ZFS_OK)
            {
                COPY_ZFS_SYSIO_ATTR(&attr, &stbuf);
            }
        }
    }
    else
    {
        SYSIO_IOFWD_FABORT();
        abort();
    }

    /* if error */
    if (err) {
        if (iino)
        {
            iino->ii_attrtim = 0;
            iino->ii_handletim = 0;
        }
        SYSIO_IOFWD_FEXIT();
        return -zfs_error_to_sysio_error(err);
    }

    if (iino) {
        iino->ii_attrtim = t + 30;
        iino->ii_handletim = t + 30;
        SYSIO_COPY_STAT(&stbuf, &ino->i_stbuf);

        /* valid file handle? copy it into the inode */
        if(fhandle_set)
        {
            iino->ii_handlevalid = 1;
            memcpy(&(iino->handle.data[0]), &(fhandle.data[0]), sizeof(zoidfs_handle_t));
        }
        if (buf)
        {
            *buf = ino->i_stbuf;
        }
        SYSIO_IOFWD_FEXIT();
        return 0;
    }
    if (!buf)
    {
        SYSIO_IOFWD_FEXIT();
        return 0;
    }
    SYSIO_COPY_STAT(&stbuf, buf);
    SYSIO_IOFWD_FEXIT();

    return 0;
}

/*
 * create a new inode for the zoidfs object
 */
static struct inode * iofwd_i_new(struct filesys *fs, time_t expiration, struct intnl_stat *buf)
{
    SYSIO_IOFWD_FENTER();
    struct iofwd_inode *iino;
    struct inode *ino;

    iino = malloc(sizeof(struct iofwd_inode));
    if (!iino)
    {
        SYSIO_IOFWD_FEXIT();
        return NULL;
    }
    memset(&iino->ii_ident, 0, sizeof(iino->ii_ident));
    iino->ii_seekok = 0;
    iino->ii_handlevalid = 0;
    iino->ii_attrvalid = 0;
    iino->ii_resetfpos = 0;
    iino->ii_ident.dev = buf->st_dev;
    iino->ii_ident.ino = buf->st_ino;
#ifdef HAVE_GENERATION
    iino->ii_ident.gen = buf->st_gen;
#endif
    iino->ii_fileid.fid_data = &iino->ii_ident;
    iino->ii_fileid.fid_len = sizeof(iino->ii_ident);
    iino->ii_fd = -1;
    iino->ii_oflags = 0;
    iino->ii_nopens = 0;
    iino->ii_fpos = 0;
    iino->ii_attrtim = expiration;
    iino->ii_handletim = 0;
    memset(&iino->handle, 0, sizeof(zoidfs_handle_t));
    ino = _sysio_i_new(fs, &iino->ii_fileid, buf, 0, &iofwd_i_ops, iino);
    if (!ino)
        free(iino);
    SYSIO_IOFWD_FEXIT();
    return ino;
}

static int iofwd_fsswop_mount(const char *source,
            unsigned flags,
            const void *data,
            struct pnode *tocover,
            struct mount **mntp)
{
    SYSIO_IOFWD_FENTER();
    int err;
    struct nameidata nameidata;
    struct mount *mnt;
    struct iofwd_filesystem *iofwdfs;
    struct inode *rootino;
    struct filesys *fs;
    unsigned long ul;
    char * opts;
    char * cp;
    int len;
    time_t t;
    struct intnl_stat stbuf;
    static struct option_value_info v[] = {
        { "atimo",  "30" },
        { NULL,     NULL }
    };

    /*
     * Caller must use fully qualified path names when specifying
     * the source.
     */
    if (*source != '/')
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOENT;
    }

    opts = NULL;
    if (data && (len = strlen((char *)data))) {
        opts = malloc(len + 1);
        if (!opts)
        {
            SYSIO_IOFWD_FEXIT();
            return -ENOMEM;
        }
        (void )strcpy(opts, data);
        if (_sysio_get_args(opts, v) - opts != (ssize_t )len)
        {
            SYSIO_IOFWD_FEXIT();
            return -EINVAL;
        }
    }
    ul = strtoul(v[0].ovi_value, &cp, 0);
    if (*cp != '\0' || ul >= UINT_MAX)
        return -EINVAL;
    if (opts) {
        free(opts);
        opts = NULL;
    }

    fs = NULL;
    mnt = NULL;
    rootino = NULL;

    iofwdfs = malloc(sizeof(struct iofwd_filesystem));
    if (!iofwdfs) {
        err = -ENOMEM;
        goto error;
    }
    iofwdfs->iofwdfs_atimo = ul;
    if ((unsigned long)iofwdfs->iofwdfs_atimo != ul) {
        err = -EINVAL;
        goto error;
    }
    fs = _sysio_fs_new(&iofwd_inodesys_ops, 0, iofwdfs);
    if (!fs) {
        err = -ENOMEM;
        goto error;
    }

    /*
     * Get root i-node.
     */
    t = _SYSIO_LOCAL_TIME();
    err = iofwd_stat("/", NULL, 0, &stbuf);
    if (err)
        goto error;

    rootino = iofwd_i_new(fs, t + FS2IOFWDFS(fs)->iofwdfs_atimo, &stbuf);
    if (!rootino) {
        err = -ENOMEM;
        goto error;
    }

    /* init the root handle */
    I2II(rootino)->ii_handlevalid = 0;
    I2II(rootino)->ii_attrtim = t;
    I2II(rootino)->ii_handletim = t;

    /*
     * Have path-node specified by the given source argument. Let the
     * system finish the job, now.
     */
    static struct qstr noname = {NULL, 0, 0};
    struct pnode_base * rootpb = _sysio_pb_new(&noname, NULL, rootino);
    if (!rootpb)
    {
        err = -ENOMEM;
        goto error;
    }

    err = _sysio_mounti(fs, rootino, flags, tocover, &mnt);
    if (!err) {
        *mntp = mnt;
    }
    SYSIO_IOFWD_FEXIT();
    return err;

error:
    if (mnt) {
        if (_sysio_do_unmount(mnt) != 0)
        {
            SYSIO_IOFWD_FABORT();
            abort();
        }
        rootino = NULL;
        fs = NULL;
        iofwdfs = NULL;
    }
    if (rootino)
        I_RELE(rootino);
    if (fs) {
        FS_RELE(fs);
        iofwdfs = NULL;
    }
    if (iofwdfs)
        free(iofwdfs);
    if (opts)
        free(opts);

    SYSIO_IOFWD_FEXIT();
    return err;
}

int _iofwd_sysio_drv_init_all()
{
    SYSIO_IOFWD_FENTER();
    extern int _sysio_native_init(void);

    /*
     * Only use the native sysio driver
     */
    int (*drvinits[])(void) = {
            NULL
    };

    int (**f)(void);
    int err;

    err = 0;
    f = drvinits;
    while (*f) {
        err = (**f++)();
        if (err)
        {
            SYSIO_IOFWD_FEXIT();
            return err;
        }
    }
    SYSIO_IOFWD_FEXIT();
    return 0;
}

/*
 * initialize the zoidfs namespace for libsysio
 * This function always runs before main() if a zoidfs libsysio
 * function is refrenced in the main() program. This
 * includes overloaded IO syscalls
 */
//void __attribute__((constructor)) _iofwd_sysio_startup(void)
void _iofwd_sysio_startup(void)
{
    SYSIO_IOFWD_FENTER();
    extern int _sysio_init(void);
    extern int _sysio_boot(const char *, const char *);

    char * driver = NULL;
    driver = getenv("ZOIDFS_SYSIO_SERVER_DRIVER");
    
    /*
     * start zoidfs
     */
    if(driver == NULL)
    {
        zoidfs_init();

        /*
         * sysio init
         */
        char * arg = NULL;
        int err = _sysio_init();
        if (err)
        {
            SYSIO_IOFWD_FEXIT();
            return;
        }

        /*
         * init sysio drivers
         */
        err = _iofwd_sysio_drv_init_all();
        if (err)
        {
            SYSIO_IOFWD_FEXIT();
            return;
        }

        /*
         * init sysio iofwd device
         */
        err = _iofwd_sysio_init();
        if (err)
        {
            SYSIO_IOFWD_FEXIT();
            return;
        }

        /*
         * Setup the sysio namespace and mounts for zoidfs... setup iofwd mount
         * at /
         * ... boot 
         */
        arg = "{mnt,dev=\"zoidfs:/\",dir=/,fl=2}";
        err = _sysio_boot("namespace", arg);
        if (err)
        {
            SYSIO_IOFWD_FEXIT();
            return;
        }
    }
    SYSIO_IOFWD_FEXIT();
}

int _iofwd_sysio_init()
{
	int	err;

    SYSIO_IOFWD_FENTER();

    err = _sysio_fssw_register("zoidfs", &iofwd_fssw_ops);

    SYSIO_IOFWD_FEXIT();

	return err;
}

/*
 * invalidate an inode and erase the handle
 */
static int iofwd_i_invalid(struct inode *inop, struct intnl_stat *stat)
{
    struct iofwd_inode * iino;

    SYSIO_IOFWD_FENTER();
    /*
     * Validate passed in inode against stat struct info
     */
    iino = I2II(inop);

    if (!iino->ii_attrtim ||
        (iino->ii_ident.dev != stat->st_dev ||
         iino->ii_ident.ino != stat->st_ino ||
#ifdef HAVE_GENERATION
         iino->ii_ident.gen != stat->st_gen ||
#endif
         ((inop)->i_stbuf.st_mode & S_IFMT) != (stat->st_mode & S_IFMT)) ||
        (((inop)->i_stbuf.st_rdev != stat->st_rdev) &&
           (S_ISCHR((inop)->i_stbuf.st_mode) ||
            S_ISBLK((inop)->i_stbuf.st_mode)))) {
        iino->ii_attrtim = 0;           /* invalidate attrs */
        iino->ii_handletim = 0;         /* invalidate handle */
        memset(&iino->handle, 0, sizeof(zoidfs_handle_t)); /* erase the handle data */
        SYSIO_IOFWD_FEXIT();
        return 1;
    }
    SYSIO_IOFWD_FEXIT();
    return 0;
}

static struct inode * iofwd_iget(struct filesys *fs, time_t expire, struct intnl_stat *stbp)
{
    struct inode *ino;
    struct iofwd_inode_identifier ident;
    struct file_identifier fileid;

    SYSIO_IOFWD_FENTER();
    memset(&ident, 0, sizeof(ident));
    ident.dev = stbp->st_dev;
    ident.ino = stbp->st_ino;
#ifdef HAVE_GENERATION
    ident.gen = stbp->st_gen;
#endif
    fileid.fid_data = &ident;
    fileid.fid_len = sizeof(ident);
    ino = _sysio_i_find(fs, &fileid);
    if (ino) {
        ino->i_stbuf = *stbp;
        I2II(ino)->ii_attrtim = expire;
        SYSIO_IOFWD_FEXIT();
        return ino;
    }
    SYSIO_IOFWD_FEXIT();
    return iofwd_i_new(fs, expire, stbp);
}

static int iofwd_ibind(struct filesys *fs, char *path, time_t t, struct inode **inop)
{
    struct intnl_stat ostbuf, stbuf;
    int err;
    struct inode *ino;

    SYSIO_IOFWD_FENTER();

    if (*inop)
        ostbuf = (*inop)->i_stbuf;

    err = iofwd_stat(path, *inop, t, &stbuf);
    if (err)
    {
        SYSIO_IOFWD_FEXIT();
        return err;
    }

    /* 
     * Validate?
     */
    if (*inop) {
        if (!iofwd_i_invalid(*inop, &ostbuf))
        {
            SYSIO_IOFWD_FEXIT();
            return 0;
        }
        /*
         * Invalidate.
         */
        _sysio_i_undead(*inop);
        *inop = NULL;
    }

    if (!(ino = iofwd_iget(fs, t + FS2IOFWDFS(fs)->iofwdfs_atimo, &stbuf)))
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    *inop = ino;
    SYSIO_IOFWD_FEXIT();
    return 0;
}

/*
 * zoidfs call for open()
 */
static int iofwd_inop_open(struct pnode *pno, int flags, mode_t mode)
{
    struct iofwd_inode * iino;
    char    *path;
    struct inode *ino;
    int fd;
    int file_exists = 0;
    int append_ii_fpos = 0;
    zoidfs_handle_t fhandle;
    int err = 0;

    SYSIO_IOFWD_FENTER();

    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    if (flags & O_WRONLY) {
        /*
         * Promote write-only attempt to RW.
         */
        flags &= ~O_WRONLY;
        flags |= O_RDWR;
    }

    /* get the file handle and set exist flag */
    if(zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT) == ZFS_OK)
    {
        file_exists = 1;
    }

    /*
     * create the file
     */
    if((flags & O_CREAT) == O_CREAT)
    {
        int created = 0;
        zoidfs_sattr_t sattr;

        sattr.mask = ZOIDFS_ATTR_MODE;
        sattr.mode = 0777 & mode;
        
        if((flags & O_EXCL) == O_EXCL)
        {
            /* if the O_CREATE | O_EXCL flag is violated, exit */
            if(file_exists)
            {
                SYSIO_IOFWD_FEXIT();
                return -zfs_error_to_sysio_error(ZFSERR_EXIST);
            }
        }

        if((err = zoidfs_create(NULL, NULL, path, &sattr, &fhandle, &created, ZOIDFS_NO_OP_HINT)) != ZFS_OK)
        {
            SYSIO_IOFWD_FEXIT();
            return -zfs_error_to_sysio_error(err);
        }
        file_exists = 1;
    }

    /*
     * resize the file to zero bytes if it exists
     */
    if((flags & O_TRUNC) == O_TRUNC)
    {
        if((err = zoidfs_resize(&fhandle, 0, ZOIDFS_NO_OP_HINT)) != ZFS_OK)
        {
            SYSIO_IOFWD_FEXIT();
            return -zfs_error_to_sysio_error(err);
        }
        file_exists = 1;
    }
    /*
     * append to the file
     */
    if((flags & O_APPEND) == O_APPEND)
    {
        zoidfs_attr_t attr;
        zoidfs_handle_t fhandle;

        attr.mask = ZOIDFS_ATTR_SIZE;

        /* can't append to a file that does not exist */
        if(!file_exists)
        {
            SYSIO_IOFWD_FEXIT();
            return -zfs_error_to_sysio_error(ZFSERR_NOENT);
        }
        if((err = zoidfs_getattr(&fhandle, &attr, ZOIDFS_NO_OP_HINT)) != ZFS_OK)
        {
            SYSIO_IOFWD_FEXIT();
            return -zfs_error_to_sysio_error(err);
        }
        append_ii_fpos = attr.size;
        file_exists = 1; 
    }

    /* if file still does not exist, exit */
    if(file_exists == 0) 
    {
        SYSIO_IOFWD_FEXIT();
        return -zfs_error_to_sysio_error(ZFSERR_NOENT);
    }

    /*
     * Since ZoidFS is a stateless protocol, there is no
     * open(). So, grab the file info and continue without
     * any ZoidFS communication. A zoidfs lookup should
     * have been invoked by libsysio, which should check
     * for file existance.
     *
     * We also must check for the O_CREAT flag so that we
     * create the file if it does not exist.
     */ 
    ino = pno->p_base->pb_ino;
    if (!ino && file_exists) {
        struct filesys *fs;
        int err;

        /*
         * Success but we need to return an i-node.
         */
        fs = pno->p_mount->mnt_fs;
        err = iofwd_ibind(fs, path, _SYSIO_LOCAL_TIME() + FS2IOFWDFS(fs)->iofwdfs_atimo, &ino);
        if (err) {
            if (err == -EEXIST)
            {
                SYSIO_IOFWD_FABORT();
                abort();
            }
            fd = err;
        }
    } 
    else
    {
        I_GET(ino);
    }

    free(path);
    if (fd < 0)
    {
        SYSIO_IOFWD_FEXIT();
        return -errno;
    }

    /*
     * Remember this new open.
     */
    iino = I2II(ino);
    iino->ii_nopens++;
    assert(iino->ii_nopens);
    do {
        if (iino->ii_fd >= 0) {
            if ((iino->ii_oflags & O_RDWR) ||
                (flags & (O_RDONLY|O_WRONLY|O_RDWR)) == O_RDONLY) {
                break;
            }
        }
        /*
         * Invariant; First open. Must init.
         */
        iino->ii_resetfpos = 0;
        iino->ii_fpos = append_ii_fpos;
        iino->ii_fd = fd;
        iino->ii_seekok = 1;
    } while (0);

    I_PUT(ino);

    SYSIO_IOFWD_FEXIT();
	return 0;
}

/*
 * zoidfs call for lookups 
 */
static int iofwd_inop_lookup(struct pnode *pno, struct inode **inop, struct intent *intnt, const char *path)
{
    int err = 0;
    time_t t;
    char * full_path = NULL;
    struct filesys *fs;
    struct iofwd_inode * iino;
    struct ionode * ino;

    SYSIO_IOFWD_FENTER();

    t = _SYSIO_LOCAL_TIME();

    *inop = pno->p_base->pb_ino;

    full_path = _sysio_pb_path(pno->p_base, '/');
    /* use cached values */
    if(*inop && (path || !intnt || (intnt->int_opmask & INT_GETATTR) == 0) && iofwd_attrs_valid(I2II(*inop), t) && iofwd_handle_valid(I2II(*inop), t))
    {
        SYSIO_IOFWD_FEXIT();
        return 0;
    }

    full_path = _sysio_pb_path(pno->p_base, '/');
    if(full_path)
    {
        fs = pno->p_mount->mnt_fs;
        err = iofwd_ibind(fs, full_path, t + FS2IOFWDFS(fs)->iofwdfs_atimo, inop);
        /*
         * bind to the file. if error set to NULL 
         */
        if(err)
        {
            *inop = NULL;
        }
        free(full_path);
        full_path = NULL;
    }

    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * zoidfs call for getattr calls 
 */
static int iofwd_inop_getattr(struct pnode *pno, struct intnl_stat *stbuf)
{
    SYSIO_IOFWD_FENTER();

    int err = 0;
    char    *path;
    struct inode *ino;
    struct filesys *fs;
    time_t  t;
    zoidfs_attr_t resattr;
    zoidfs_handle_t fhandle;

    /* get the path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    /* get the file handle */
    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err == ZFS_OK)
    {
        /* get the attrs for the file handle */
        resattr.mask = ZOIDFS_ATTR_ALL; 
        err = zoidfs_getattr(&fhandle, &resattr, ZOIDFS_NO_OP_HINT);
        if(err == ZFS_OK)
        {
            COPY_ZFS_SYSIO_ATTR(&resattr, stbuf);
        }
    }

    /* cleanup */
    free(path);

    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * zoidfs call for setattr calls 
 */
static int iofwd_inop_setattr(struct pnode *pno, unsigned mask, struct intnl_stat *stbuf)
{
    SYSIO_IOFWD_FENTER();

    int err = 0;
    char    *path;
    struct inode *ino;
    struct filesys *fs;
    time_t  t;
    zoidfs_attr_t attr;
    zoidfs_sattr_t sattr;
    zoidfs_handle_t fhandle;

    /* get the path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    /* get the file handle */
    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err == ZFS_OK)
    {

        /* get the original file attributes */
        sattr.mask = 0;

        /* convert the sysio mask to zoidfs */
        if(mask & SETATTR_MODE)
        {
            sattr.mask = sattr.mask | ZOIDFS_ATTR_MODE;
            sattr.mode = stbuf->st_mode & 0777;
        }
        else
        {
            sattr.mode = 0;
        }

        if(mask & SETATTR_UID)
        {
            sattr.mask = sattr.mask | ZOIDFS_ATTR_UID;
            sattr.uid = stbuf->st_uid;
        }
        else
        {
            sattr.uid = 0;
        }
    
        if(mask & SETATTR_GID)
        {
            sattr.mask = sattr.mask | ZOIDFS_ATTR_GID;
            sattr.uid = stbuf->st_gid;
        }   
        else
        {
            sattr.gid = 0;
        }

        if(mask & SETATTR_ATIME)
        {
            sattr.mask = sattr.mask | ZOIDFS_ATTR_ATIME;
            sattr.atime.seconds = stbuf->st_atime;
            sattr.atime.nseconds = 0;
        }
        else
        {
            sattr.atime.seconds = 0;
            sattr.atime.nseconds = 0;
        }

        if(mask & SETATTR_MTIME)
        {
            sattr.mask = sattr.mask | ZOIDFS_ATTR_MTIME;
            sattr.mtime.seconds = stbuf->st_mtime;
            sattr.mtime.nseconds = 0;
        }
        else
        {
            sattr.mtime.seconds = 0;
            sattr.mtime.nseconds = 0;
        }

        if(sattr.mask)
        {
            sattr.size = 0; /* only set the size using resize */
            attr.mask = sattr.mask;
            err = zoidfs_setattr(&fhandle, &sattr, &attr, ZOIDFS_NO_OP_HINT);
            if(err != ZFS_OK)
            {
                SYSIO_IOFWD_FEXIT();
                return zfs_error_to_sysio_error(err);
            }
        }

        if (mask & SETATTR_LEN)
        {
            err = zoidfs_resize(&fhandle, stbuf->st_size, ZOIDFS_NO_OP_HINT);
            if(err != ZFS_OK)
            {
                SYSIO_IOFWD_FEXIT();
                return zfs_error_to_sysio_error(err);
            }
            attr.size = stbuf->st_size;
            attr.mask = ZOIDFS_ATTR_SIZE;
        }
        COPY_ZFS_SYSIO_ATTR(&attr, stbuf);
    }

    /* cleanup */
    if(path)
    {
        free(path);
        path = NULL;
    }

    SYSIO_IOFWD_FEXIT();
	return 0;
}

/*
 * zoidfs call for dirent calls 
 */
static ssize_t iofwd_filldirentries(struct pnode *pno, _SYSIO_OFF_T *posp, char *buf, size_t nbytes)
{
    int err = 0;
    zoidfs_handle_t dhandle;
    zoidfs_dirent_cookie_t cookie = *posp;
    size_t entry_count = nbytes / sizeof(struct dirent64); /*entry count is the max dirents that can fit in the user buffer */
    const size_t max_entry_count = entry_count;
    zoidfs_dirent_t * entries = NULL;
    char * path = NULL;
    struct dirent64 ** outdp = (struct dirent64 **)&buf;
    int i = 0;

    SYSIO_IOFWD_FENTER();

    /* get the directory handle */
    path = _sysio_pb_path(pno->p_base, '/');
    err = zoidfs_lookup(NULL, NULL, path, &dhandle, ZOIDFS_NO_OP_HINT);
    if(err != ZFS_OK)
    {
        SYSIO_IOFWD_FEXIT();
        return -zfs_error_to_sysio_error(err);
    }

    /* setup the internal buffers */
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));

    /* get the dir entries */
    err = zoidfs_readdir(&dhandle, cookie, &entry_count, entries, 0, NULL, ZOIDFS_NO_OP_HINT);
    SYSIO_IOFWD_FINFO("ec = %i", entry_count);
    if(err != ZFS_OK)
    {
        SYSIO_IOFWD_FEXIT();
        return -zfs_error_to_sysio_error(err);
    }

    /* set the directory pointer */
    *posp = entries[entry_count - 1].cookie;

    /* copy the data from the internal buffer to the user buffer */
    uint64_t last_off = 0;
    for(i = 0 ; i < entry_count ; i++)
    {
        SYSIO_IOFWD_FINFO("entry %i", i);
        int nlen = strlen(entries[i].name);
        int rlen = sizeof(*outdp[i]) - sizeof(outdp[i]->d_name) + nlen; /* number of unused bytes in the d_name field */
        int nlen_unused = rlen;
        int j = 0;

        if (nbytes <= rlen)
            break;
        outdp[i]->d_ino = 0; /* don't know the inode number... set it to 0 */
        //outdp[i]->d_off = entries[i].cookie;
        outdp[i]->d_off = last_off;
        outdp[i]->d_reclen = (((rlen + sizeof(long))) / sizeof(long)) * sizeof(long);
        if (nbytes < outdp[i]->d_reclen)
            outdp[i]->d_reclen = rlen + 1;
        last_off = outdp[i]->d_reclen;

        /* set the dirent type */
#ifdef __USE_BSD
        switch(entries[i].attr.type)
        {
            case ZOIDFS_INVAL:
            {
                outdp[i]->d_type = DT_UNKNOWN;
                break;
            }
            case ZOIDFS_REG:
            {
                outdp[i]->d_type = DT_REG;
                break;
            }
            case ZOIDFS_DIR:
            {
                outdp[i]->d_type = DT_DIR;
                break;
            }
            case ZOIDFS_LNK:
            {
                outdp[i]->d_type = DT_LNK;
                break;
            }
            case ZOIDFS_CHR:
            {
                outdp[i]->d_type = DT_CHR;
                break;
            }
            case ZOIDFS_BLK:
            {
                outdp[i]->d_type = DT_BLK;
                break;
            }
            case ZOIDFS_FIFO:
            {
                outdp[i]->d_type = DT_FIFO;
                break;
            }
            case ZOIDFS_SOCK:
            {
                outdp[i]->d_type = DT_SOCK;
                break;
            }
            default:
            {
                outdp[i]->d_type = DT_UNKNOWN;
                break;
            }
        };
#else
        outdp[i]->d_type = 0;
#endif

        /* copy the file name */
        memcpy(outdp[i]->d_name, entries[i].name, nlen);

        /* null characters for rest of name string */
        for(j = nlen ; j < nlen_unused ; j++)
        {
            outdp[i]->d_name[j] = '\0';
        }

        SYSIO_IOFWD_FINFO("off = %lu reclen = %lu name = %s", outdp[i]->d_off, outdp[i]->d_reclen, outdp[i]->d_name);
        nbytes -= outdp[i]->d_reclen;
    }

    /* cleanup */
    if(entries)
    {
        free(entries);
        entries = NULL;
    }
    
    if(path)
    {
        free(path);
        path = NULL;
    }

    SYSIO_IOFWD_FEXIT();
	return ((char *)outdp[i] - buf);
}

/*
 * invokes zoidfs_mkdir() to create a directory on the remote host
 */
static int iofwd_inop_mkdir(struct pnode *pno, mode_t mode)
{
    SYSIO_IOFWD_FENTER();

    char    *path;
    int err;
    zoidfs_sattr_t sattr;

    /* set the mode in the zoidfs attribute */
    sattr.mask = ZOIDFS_ATTR_MODE;
    sattr.mode = 0777 & mode; 

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    /* invoke the zoidfs mkdir call */
    err = zoidfs_mkdir(NULL, NULL, path, &sattr, NULL, ZOIDFS_NO_OP_HINT);

    /* cleanup */    
    free(path);

    SYSIO_IOFWD_FEXIT();
    return err;
}

/*
 * zoidfs call / mapping for posix rmdir() 
 */
static int iofwd_inop_rmdir(struct pnode *pno)
{
    SYSIO_IOFWD_FENTER();

    char * path;
    int err;
    zoidfs_handle_t fhandle;
    zoidfs_attr_t attr;

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    /* check that this is a directory... if not exit with an error */
    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err != ZFS_OK)
    {
        SYSIO_IOFWD_FEXIT();
        return err;
    }

    /* get the file attributes */
    attr.mask = ZOIDFS_ATTR_MODE;
    err = zoidfs_getattr(&fhandle, &attr, ZOIDFS_NO_OP_HINT);
    if(err != ZFS_OK)
    {
        SYSIO_IOFWD_FEXIT();
        return err;
    } 

    /* Is this a directory */
    if(attr.type != ZOIDFS_DIR)
    {
        return ENOTDIR;
    }

    /* invoke the zoidfs remove call to remove the directory */
    err = zoidfs_remove(NULL, NULL, path, NULL, ZOIDFS_NO_OP_HINT);
    
    /* cleanup */   
    free(path);

    SYSIO_IOFWD_FEXIT();
	return err; 
}

/*
 * zoidfs call for symlink() 
 */
static int iofwd_inop_symlink(struct pnode *pno, const char *data)
{
    SYSIO_IOFWD_FENTER();

    char * path;
    int err;
    zoidfs_handle_t fhandle;
    zoidfs_sattr_t sattr;

    sattr.mask = 0;

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    /* invoke the zoidfs remove call to remove the directory */
    err = zoidfs_symlink(NULL, NULL, data, NULL, NULL, path, &sattr, NULL, NULL, ZOIDFS_NO_OP_HINT);

    /* cleanup */
    free(path);

    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * zoidfs call for readlink() 
 */
static int iofwd_inop_readlink(struct pnode *pno, char *buf, size_t bufsiz)
{
    SYSIO_IOFWD_FENTER();

    char * path;
    int err;
    zoidfs_handle_t fhandle;

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err == ZFS_OK)
    {
        /* run the zoidfs readlink() */
        err = zoidfs_readlink(&fhandle, buf, bufsiz, ZOIDFS_NO_OP_HINT);
    }

    /* cleanup */
    free(path);

    SYSIO_IOFWD_FEXIT();
	return bufsiz;
}

/*
 * zoidfs call for close() 
 */
static int iofwd_inop_close(struct pnode *pno)
{
    SYSIO_IOFWD_FENTER();

    struct inode *ino;
    struct iofwd_inode *iino;
    int err;
    char * path;

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    ino = pno->p_base->pb_ino;
    assert(ino);
    iino = I2II(ino);
    if (iino->ii_fd < 0)
    {
        SYSIO_IOFWD_FABORT();
        abort();
    }

    assert(iino->ii_nopens);
    if (--iino->ii_nopens) 
    {
        zoidfs_handle_t fhandle;

        err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
        /*
         * Force a zoidfs commit to make sure all 
         * outstanding data buffers are flushed
         */
        zoidfs_commit(&fhandle, ZOIDFS_NO_OP_HINT);
    
        /* cleanup */
        free(path);

        SYSIO_IOFWD_FEXIT();
        return 0;
    }

    iino->ii_fd = -1;
    iino->ii_resetfpos = 0;
    iino->ii_fpos = 0;

    /* cleanup */
    free(path);

    SYSIO_IOFWD_FEXIT();
	return 0;
}

/*
 * zoidfs call for link()
 */
static int iofwd_inop_link(struct pnode *old, struct pnode *new)
{
    SYSIO_IOFWD_FENTER();

    char * old_path, * new_path;
    int err = 0;

    /* get the path of the old file */
    old_path = _sysio_pb_path(old->p_base, '/');
    if (!old_path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    /* get the path of the new file */
    new_path = _sysio_pb_path(new->p_base, '/');
    if (!new_path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    err = zoidfs_link(NULL, NULL, old_path, NULL, NULL, new_path, NULL, NULL, ZOIDFS_NO_OP_HINT);

    /* cleanup */
    free(old_path);
    free(new_path);

    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * unlink a file
 * This function invokes the zoidfs_remove() function to issue
 * an IO fowarding remove request to the IO forwarding server.
 */
static int iofwd_inop_unlink(struct pnode *pno)
{
    SYSIO_IOFWD_FENTER();
    char    *path;
    int err = 0;

    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return -ENOMEM;
    }

    err = zoidfs_remove(NULL, NULL, path, NULL, ZOIDFS_NO_OP_HINT);
    
    free(path);

    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * zoidfs call for rename() 
 */
static int iofwd_inop_rename(struct pnode *old, struct pnode *new)
{
    SYSIO_IOFWD_FENTER();

    int err;
    char    *opath, *npath;

    opath = _sysio_pb_path(old->p_base, '/');
    npath = _sysio_pb_path(new->p_base, '/');
    if (!(opath && npath)) {
        err = -ENOMEM;
        goto out;
    }

    /*
     * invoke zoidfs_rename using the fullpaths
     */
    err = zoidfs_rename(NULL, NULL, opath, NULL, NULL, npath, NULL, NULL, ZOIDFS_NO_OP_HINT);

out:
    if (opath)
        free(opath);
    if (npath)
        free(npath);

    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * zoidfs call for read() 
 */
static int iofwd_inop_read(struct ioctx *ioctx)
{
    SYSIO_IOFWD_FENTER();

    int err = 0;
    zoidfs_handle_t fhandle;
    char * path;
    void ** mem_starts;
    size_t * mem_sizes;
    uint64_t * file_starts;
    uint64_t * file_sizes;
    int i = 0;

    struct inode *ino;
    struct iofwd_inode *iino;

    ino = ioctx->ioctx_pno->p_base->pb_ino;
    iino = I2II(ino);

    /* get the full path of the file from the sysio pnode */
    path = _sysio_pb_path(ioctx->ioctx_pno->p_base, '/');
    if(!path)
    {
        SYSIO_IOFWD_FEXIT();
        return ENOMEM;
    }
   
    /* lookup the file by the full path to get the handle */ 
    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err != ZFS_OK)
    {
        SYSIO_IOFWD_FEXIT();
        return err;
    }

    /* allocate the zoidfs buffers */
    mem_starts = (void **)malloc(sizeof(void *) * ioctx->ioctx_iovlen);
    mem_sizes = (size_t *)malloc(sizeof(size_t) * ioctx->ioctx_iovlen);
    file_starts = (uint64_t *)malloc(sizeof(uint64_t) * ioctx->ioctx_xtvlen);
    file_sizes = (uint64_t *)malloc(sizeof(uint64_t) * ioctx->ioctx_xtvlen);
 
    /* iovec data conversions */
    for(i = 0 ; i < ioctx->ioctx_iovlen ; i+=1)
    {
        mem_starts[i] = (void *)ioctx->ioctx_iov[i].iov_base;
        mem_sizes[i] = (size_t)ioctx->ioctx_iov[i].iov_len;
    }

    /* xtvec data conversions */
    uint64_t max_pos = 0;
    for(i = 0 ; i < ioctx->ioctx_xtvlen ; i+=1)
    {
        file_starts[i] = (uint64_t)ioctx->ioctx_xtv[i].xtv_off + iino->ii_fpos;
        file_sizes[i] = (uint64_t)ioctx->ioctx_xtv[i].xtv_len;
        SYSIO_IOFWD_FINFO("file_starts[%i] = %lu file_sizes[%i] = %lu", i, file_starts[i], i, file_sizes[i]);
        if(file_starts[i] + file_sizes[i] > max_pos)
        {
            max_pos = file_starts[i] + file_sizes[i];
        }
    }

    /* invoke the zoidfs_read() */
    err = zoidfs_read(&fhandle, ioctx->ioctx_iovlen, mem_starts, mem_sizes, ioctx->ioctx_xtvlen, file_starts, file_sizes, ZOIDFS_NO_OP_HINT);

    /* update the file pointer */
    if (!err)
    {
        iino->ii_fpos = max_pos;
    }

    /* cleanup */
    free(mem_starts);
    free(mem_sizes);
    free(file_starts);
    free(file_sizes);    
    free(path);

    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * zoidfs call for write() 
 */
static int iofwd_inop_write(struct ioctx *ioctx)
{
    SYSIO_IOFWD_FENTER();

    int err = 0;
    zoidfs_handle_t fhandle;
    char * path;
    void ** mem_starts;
    size_t * mem_sizes;
    uint64_t * file_starts;
    uint64_t * file_sizes;
    int i = 0;

    struct inode *ino;
    struct iofwd_inode *iino;

    ino = ioctx->ioctx_pno->p_base->pb_ino;
    iino = I2II(ino);

    /* get the full path of the file from the sysio pnode */
    path = _sysio_pb_path(ioctx->ioctx_pno->p_base, '/');
    if(!path)
    {
        SYSIO_IOFWD_FEXIT();
        return ENOMEM;
    }

    /* lookup the file by the full path to get the handle */
    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err != ZFS_OK)
    {
        SYSIO_IOFWD_FEXIT();
        return err;
    }

    /* allocate the zoidfs buffers */
    mem_starts = (void **)malloc(sizeof(void *) * ioctx->ioctx_iovlen);
    mem_sizes = (size_t *)malloc(sizeof(size_t) * ioctx->ioctx_iovlen);
    file_starts = (uint64_t *)malloc(sizeof(uint64_t) * ioctx->ioctx_xtvlen);
    file_sizes = (uint64_t *)malloc(sizeof(uint64_t) * ioctx->ioctx_xtvlen);

    /* iovec data conversions */
    for(i = 0 ; i < ioctx->ioctx_iovlen ; i+=1)
    {
        mem_starts[i] = (void *)ioctx->ioctx_iov[i].iov_base;
        mem_sizes[i] = (size_t)ioctx->ioctx_iov[i].iov_len;
    }

    /* xtvec data conversions */
    uint64_t max_pos = 0; 
    for(i = 0 ; i < ioctx->ioctx_xtvlen ; i+=1)
    {
        file_starts[i] = (uint64_t)ioctx->ioctx_xtv[i].xtv_off + iino->ii_fpos;
        file_sizes[i] = (uint64_t)ioctx->ioctx_xtv[i].xtv_len;
        if(file_starts[i] + file_sizes[i] > max_pos)
        {
            max_pos = file_starts[i] + file_sizes[i];
        }
        SYSIO_IOFWD_FINFO("file_starts[%i] = %lu file_sizes[%i] = %lu", i, file_starts[i], i, file_sizes[i]);
    }

    /* invoke the zoidfs_read() */
    err = zoidfs_write(&fhandle, ioctx->ioctx_iovlen, (const void **)mem_starts, mem_sizes, ioctx->ioctx_xtvlen, file_starts, file_sizes, ZOIDFS_NO_OP_HINT);

    /* update the file pointer */
    if (!err)
    {
        iino->ii_fpos = max_pos;
    }

    /* cleanup */
    free(mem_starts);
    free(mem_sizes);
    free(file_starts);
    free(file_sizes);
    free(path);

    SYSIO_IOFWD_FEXIT();
	return 0;
}

static _SYSIO_OFF_T iofwd_inop_pos(struct pnode *pno, _SYSIO_OFF_T off)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FEXIT();
	return 0;
}

/*
 * zoidfs call to determine if the IO completed...
 * no async IO support, so it's done when read()
 * write() return. Always return 1. 
 */
static int iofwd_inop_iodone(struct ioctx *ioctx)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FEXIT();
	return 1;
}

/*
 * zoidfs call for fcntl() 
 */
static int iofwd_inop_fcntl(struct pnode *pno, int cmd, va_list ap, int *rtn)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FEXIT();
	return 0;
}

static int iofwd_inop_sync(struct pnode *pno)
{
    SYSIO_IOFWD_FENTER();
    zoidfs_handle_t fhandle;
    int err = 0;
    char * path;

    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return ENOMEM;
    }

    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err != ZFS_OK)
    {
        free(path);
        SYSIO_IOFWD_FEXIT();
        return err;
    }
    err = zoidfs_commit(&fhandle, ZOIDFS_NO_OP_HINT);

    free(path);
    SYSIO_IOFWD_FEXIT();
	return err;
}

static int iofwd_inop_datasync(struct pnode *pno)
{
    SYSIO_IOFWD_FENTER();
    zoidfs_handle_t fhandle;
    int err = 0;
    char * path;

    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_IOFWD_FEXIT();
        return ENOMEM;
    }

    err = zoidfs_lookup(NULL, NULL, path, &fhandle, ZOIDFS_NO_OP_HINT);
    if(err != ZFS_OK)
    {
        free(path);
        SYSIO_IOFWD_FEXIT();
        return err;
    }
    err = zoidfs_commit(&fhandle, ZOIDFS_NO_OP_HINT);

    free(path);
    SYSIO_IOFWD_FEXIT();
	return err;
}

/*
 * zoidfs call for ioctl() 
 */
static int iofwd_inop_ioctl(struct pnode *pno, unsigned long int request, va_list ap)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FEXIT();
	return ENOTTY;
}

/* not supported by zoidfs */
static int iofwd_inop_mknod(struct pnode *pno, mode_t mode, dev_t dev)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FEXIT();
	return ENOSYS;
}

#ifdef _HAVE_STATVFS
/* not supported */
static int iofwd_inop_statvfs(struct pnode *pno, struct intnl_statvfs *buf)
{
    SYSIO_IOFWD_FENTER();
    SYSIO_IOFWD_FEXIT();
	return ENOSYS;
}

#endif

/*
 * cleanup when the inop is gone
 */
static void iofwd_inop_gone(struct inode *ino)
{
    SYSIO_IOFWD_FENTER();

    /* sysio cleanup only */
    free(ino->i_private);

    SYSIO_IOFWD_FEXIT();
	return;
}

/*
 * cleanup when the fsop is gone
 */
static void iofwd_fsop_gone(struct filesys * fs)
{
    SYSIO_IOFWD_FENTER();

    /* sysio cleanup only */
    free(fs->fs_private);

    SYSIO_IOFWD_FEXIT();
}
