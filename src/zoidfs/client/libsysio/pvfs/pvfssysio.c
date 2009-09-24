/*
        fprintf(stderr, "%s %s\n", *base, *component);
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

#include "pvfs2-sysint.h"
#include "pvfs2-util.h"
#include "pvfs2-types.h"

#include "pvfs2.h"

#include <string.h>
#include <strings.h>

/*
 * Debug and trace facilities
 */


//#define DEBUG_SYSIO_PVFS 
#ifdef DEBUG_SYSIO_PVFS
#define SYSIO_PVFS_FENTER()                                                             \
do{                                                                                     \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i ENTER\n", __func__, __FILE__, __LINE__);   \
}while(0)

#define SYSIO_PVFS_FEXIT()                                                              \
do{                                                                                     \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i EXIT\n", __func__, __FILE__, __LINE__);    \
}while(0)

#define SYSIO_PVFS_FABORT()                                                             \
do{                                                                                     \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i ABORT\n", __func__, __FILE__, __LINE__);   \
}while(0)

#define SYSIO_PVFS_FERRVAL(...)                                                                         \
do{                                                                                                     \
    char __buffer[4096];                                                                                \
    sprintf(__buffer, ##__VA_ARGS__);                                                                   \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i ERRVAL, %s\n", __func__, __FILE__, __LINE__, __buffer);    \
}while(0)

#else
#define SYSIO_PVFS_FENTER()    /* not in debug mode... do nothing */
#define SYSIO_PVFS_FEXIT()     /* not in debug mode... do nothing */
#define SYSIO_PVFS_FABORT()    /* not in debug mode... do nothing */
#define SYSIO_PVFS_FERRVAL(...)    /* not in debug mode... do nothing */
#endif

#define SYSIO_PVFS_FINFO(...)                                                                          \
do{                                                                                                     \
    char __buffer[4096];                                                                                \
    sprintf(__buffer, ##__VA_ARGS__);                                                                   \
    fprintf(stderr, "%s, FILE=%s, LINE# = %i INFO, %s\n", __func__, __FILE__, __LINE__, __buffer);      \
}while(0)

int _pvfs_sysio_init();
int _pvfs_sysio_drv_init_all();

static int pvfs_fsswop_mount(const char *source, unsigned flags, const void *data, struct pnode *tocover, struct mount **mntp);
struct fssw_ops pvfs_fssw_ops = {pvfs_fsswop_mount};

static int create_internal_namespace(const void *data);

static void pvfs_fsop_gone(struct filesys *fs);
static struct filesys_ops pvfs_inodesys_ops = {pvfs_fsop_gone,};

static struct inode * pvfs_i_new(struct filesys *fs, time_t expiration, struct intnl_stat *buf);
static struct inode * pvfs_iget(struct filesys *fs, time_t expire, struct intnl_stat *stbp);
static int pvfs_ibind(struct filesys *fs, char *path, time_t t, struct inode **inop);
static int pvfs_i_invalid(struct inode *inop, struct intnl_stat *stat);

/*
 * PVFS operations mapped to the driver
 */
static int pvfs_inop_lookup(struct pnode *pno, struct inode **inop, struct intent *intnt, const char *path);
static int pvfs_inop_getattr(struct pnode *pno, struct intnl_stat *stbuf);
static int pvfs_inop_setattr(struct pnode *pno, unsigned mask, struct intnl_stat *stbuf);
static ssize_t pvfs_filldirentries(struct pnode *pno, _SYSIO_OFF_T *posp, char *buf, size_t nbytes);
static int pvfs_inop_mkdir(struct pnode *pno, mode_t mode);
static int pvfs_inop_rmdir(struct pnode *pno);
static int pvfs_inop_symlink(struct pnode *pno, const char *data);
static int pvfs_inop_readlink(struct pnode *pno, char *buf, size_t bufsiz);
static int pvfs_inop_open(struct pnode *pno, int flags, mode_t mode);
static int pvfs_inop_close(struct pnode *pno);
static int pvfs_inop_link(struct pnode *old, struct pnode *new);
static int pvfs_inop_unlink(struct pnode *pno);
static int pvfs_inop_rename(struct pnode *old, struct pnode *new);
static int pvfs_inop_read(struct ioctx *ioctx);
static int pvfs_inop_write(struct ioctx *ioctx);
static _SYSIO_OFF_T pvfs_inop_pos(struct pnode *pno, _SYSIO_OFF_T off);
static int pvfs_inop_iodone(struct ioctx *ioctx);
static int pvfs_inop_fcntl(struct pnode *pno, int cmd, va_list ap, int *rtn);
static int pvfs_inop_sync(struct pnode *pno);
static int pvfs_inop_datasync(struct pnode *pno);
static int pvfs_inop_ioctl(struct pnode *pno, unsigned long int request, va_list ap);
static int pvfs_inop_mknod(struct pnode *pno, mode_t mode, dev_t dev);
#ifdef _HAVE_STATVFS
static int pvfs_inop_statvfs(struct pnode *pno, struct intnl_statvfs *buf);
#endif
static void pvfs_inop_gone(struct inode *ino);

static char mps_root[4096];
/*
 */
static struct inode_ops pvfs_i_ops = {
    inop_lookup: pvfs_inop_lookup,
    inop_getattr: pvfs_inop_getattr,
    inop_setattr: pvfs_inop_setattr,
    inop_filldirentries: pvfs_filldirentries,
    inop_filldirentries2: NULL,
    inop_mkdir: pvfs_inop_mkdir,
    inop_rmdir: pvfs_inop_rmdir,
    inop_symlink: pvfs_inop_symlink,
    inop_readlink: pvfs_inop_readlink,
    inop_open: pvfs_inop_open,
    inop_close: pvfs_inop_close,
    inop_link: pvfs_inop_link,
    inop_unlink: pvfs_inop_unlink,
    inop_rename: pvfs_inop_rename,
    inop_read: pvfs_inop_read,
    inop_write: pvfs_inop_write,
    inop_pos: pvfs_inop_pos,
    inop_iodone: pvfs_inop_iodone,
    inop_fcntl: pvfs_inop_fcntl,
    inop_sync: pvfs_inop_sync,
    inop_datasync: pvfs_inop_datasync,
    inop_ioctl: pvfs_inop_ioctl,
    inop_mknod: pvfs_inop_mknod,
#ifdef _HAVE_STATVFS
    inop_statvfs: pvfs_inop_statvfs,
#endif
    inop_perms_check: _sysio_p_generic_perms_check,
    inop_gone: pvfs_inop_gone
};


static inline int pvfs_error_to_sysio_error(int val)
{
   if (val == 0)
      return 0;

    switch(-val)
    {
        case PVFS_EINVAL:
        {
            SYSIO_PVFS_FERRVAL("EINVAL encountered");
            return EINVAL;
        }
        case PVFS_ENOMEM:
        {
            SYSIO_PVFS_FERRVAL("ENOMEM encountered");
            return ENOMEM;
        }
        case PVFS_EBADF:
        {
            SYSIO_PVFS_FERRVAL("EBADF encountered");
            return EBADF;
        }
        case PVFS_EPERM:
        {
            SYSIO_PVFS_FERRVAL("EPERM encountered");
            return EPERM;
        }
        case PVFS_ENOENT:
        {
            SYSIO_PVFS_FERRVAL("ENOENT encountered");
            return ENOENT;
        }
        case PVFS_EIO:
        {
            SYSIO_PVFS_FERRVAL("EIO encountered");
            return EIO;
        }
        case PVFS_ENXIO:
        {
            SYSIO_PVFS_FERRVAL("ENXIO encountered");
            return ENXIO;
        }
        case PVFS_EACCES:
        {
            SYSIO_PVFS_FERRVAL("EACCES encountered");
            return EACCES;
        }
        case PVFS_EEXIST:
        {
            SYSIO_PVFS_FERRVAL("EEXIST encountered");
            return EEXIST;
        }
        case PVFS_ENODEV:
        {
            SYSIO_PVFS_FERRVAL("ENODEV encountered");
            return ENODEV;
        }
        case PVFS_ENOTDIR:
        {
            SYSIO_PVFS_FERRVAL("ENOTDIR encountered");
            return ENOTDIR;
        }
        case PVFS_EISDIR:
        {
            SYSIO_PVFS_FERRVAL("EISDIR encountered");
            return EISDIR;
        }
        case PVFS_EFBIG:
        {
            SYSIO_PVFS_FERRVAL("EFBIG encountered");
            return EFBIG;
        }
        case PVFS_ENOSPC:
        {
            SYSIO_PVFS_FERRVAL("ENOSPC encountered");
            return ENOSPC;
        }
        case PVFS_EROFS:
        {
            SYSIO_PVFS_FERRVAL("EROFS encountered");
            return EROFS;
        }
        case PVFS_ENAMETOOLONG:
        {
            SYSIO_PVFS_FERRVAL("ENAMETOOLONG encountered");
            return ENAMETOOLONG;
        }
        case PVFS_ENOTEMPTY:
        {
            SYSIO_PVFS_FERRVAL("ENOTEMPTY encountered");
            return ENOTEMPTY;
        }
        default:
        {
            SYSIO_PVFS_FERRVAL("unknown error encounter %i", val);
        }
    }
    return -val;
}

/* conver pvfs attrs to sysio attrs */
static inline int pvfs_attr_to_sysio_attr(const PVFS_sys_attr * pattr, const PVFS_object_ref * pref, struct intnl_stat * sstat)
{
    sstat->st_uid = pattr->owner;
    sstat->st_gid = pattr->group;
    sstat->st_atime = pattr->atime;
    sstat->st_mtime = pattr->mtime;
    sstat->st_ctime = pattr->ctime;
    sstat->st_size = pattr->size;
    sstat->st_nlink = 2;    /* no hard link support in pvfs */
    sstat->st_rdev = 0;     /* no support for char or block devices */
    sstat->st_dev = pref->fs_id;
    sstat->st_ino = pref->handle;
    sstat->st_blksize = pattr->blksize;

    /* if this is a file, calculate the block size */
    if(pattr->blksize > 0 && pattr->objtype == PVFS_TYPE_METAFILE)
    {
        if(pattr->size % pattr->blksize == 0)
        {
            sstat->st_blocks = pattr->size / pattr->blksize;
        }
        else
        {
            sstat->st_blocks = ((int)(pattr->size / pattr->blksize)) + 1;
        }
    }
    /* otherwise, set to 0 */
    else
    {
        sstat->st_blocks = 0;
    }
    
    /* set the file mode field */
    sstat->st_mode = pattr->perms & 0777;
    if(pattr->objtype == PVFS_TYPE_METAFILE)
    {
        sstat->st_mode = sstat->st_mode | S_IFREG;
    }
    else if(pattr->objtype == PVFS_TYPE_DIRECTORY)
    {
        sstat->st_mode = sstat->st_mode | S_IFDIR;
    }
    else if(pattr->objtype == PVFS_TYPE_SYMLINK)
    {
        sstat->st_mode = sstat->st_mode | S_IFLNK;
    }
    
    return 0;
}

/* convert sysio attrs to pvfs attrs */
static inline int sysio_attr_to_pvfs_attr(const struct intnl_stat * sstat, PVFS_sys_attr * pattr)
{
    pattr->owner = sstat->st_uid;
    pattr->group = sstat->st_gid;
    pattr->perms = sstat->st_mode & 0777;
    pattr->atime = sstat->st_atime;
    pattr->mtime = sstat->st_mtime; 
    pattr->ctime = sstat->st_ctime; 
    pattr->size  = sstat->st_size;

    pattr->link_target = NULL;
    pattr->dist_name = NULL;
    pattr->dist_params = NULL;
    pattr->dfile_count = 0;
    pattr->dirent_count = 0;
    pattr->flags = 0;

    /* set the object type field */
    if(S_ISREG(sstat->st_mode))
    {
        pattr->objtype = PVFS_TYPE_METAFILE;
    }
    else if(S_ISDIR(sstat->st_mode))
    {
        pattr->objtype = PVFS_TYPE_DIRECTORY;
    }
    else if(S_ISLNK(sstat->st_mode))
    {
        pattr->objtype = PVFS_TYPE_SYMLINK;
    }

    /* set all attrs */
    pattr->mask = PVFS_ATTR_SYS_ALL_SETABLE;

    return 0;
}

/* 
 * split the paths into a component and base path
 * the paths are split on the last / in the string...
 *  /path/to/file => base = /path/to, component => file
 *  /path => base = /path, component = NULL
 */
inline static void sysio_pvfs_split_path(const char * path,
                                         char ** base,
                                         char ** component)
{
    char * tmpstr = NULL;
    char * index = NULL;

    assert(path);

    tmpstr = strdup(path);

    index = rindex(tmpstr, '/');
    if(index)
    {
        *index = 0;
        ++index;

        *component = strdup(index);
        *base = tmpstr;
        /* The condition below will match for path such as "/file"
           (only one "/", right in front).  We make sure not to return
           an empty base in such a case.  */
        if (strlen(*base) == 0)
            strcpy(*base, "/");
    }
    else
    {
        *component = tmpstr;
        *base = NULL;
    }
}

/*
 * helper function to initialize the pvfs libsysio driver
 * This function must be called to use the driver
 *   - init PVFS interface
 *   - init sysio
 *   - init sysio drivers (native, ...)
 *   - init the pvfs sysio driver
 *   - mount / boot pvfs at mp
 */
void start_pvfs_sysio_driver(char * mp, char * pvfs_root)
{
    SYSIO_PVFS_FENTER();

    extern int _sysio_init(void);
    extern int _sysio_boot(const char *, const char *);

    char arg[4096];
    int err = 0;

    /*
     * PVFS init
     */
    err = PVFS_util_init_defaults();
    if(err < 0)
    {
        return;
    }

    /*
     * sysio init
     */
    err = _sysio_init();
    if (err)
    {
        SYSIO_PVFS_FEXIT();
        return;
    }

    /*
     * init sysio drivers
     */
    err = _pvfs_sysio_drv_init_all();
    if (err)
    {
        SYSIO_PVFS_FEXIT();
        return;
    }

    /*
     * init sysio pvfs device
     */
    err = _pvfs_sysio_init();
    if (err)
    {
        SYSIO_PVFS_FEXIT();
        return;
    }

    /* set the mps_root variable with the pvfs root mount point */
    memset(mps_root, 0, 4096);
    if(pvfs_root)
    {
        strcpy(mps_root, pvfs_root);
    }

    /*
     * boot the pvfs driver and mount it
     */
    memset(arg, 0, 4096);
    sprintf(arg, "{mnt,dev=\"pvfs:%s\",dir=/,fl=2}", mp);
    err = _sysio_boot("namespace", arg);
    if (err)
    {
        SYSIO_PVFS_FEXIT();
        return;
    }

    SYSIO_PVFS_FEXIT();
}

void shutdown_pvfs_sysio_driver(void)
{
    SYSIO_PVFS_FENTER();

    PVFS_sys_finalize();

    SYSIO_PVFS_FEXIT();
    return;
}

/*
 * Internal library data structures
 */
static struct mount * pvfs_internal_mount = NULL;

struct pvfs_filesystem {
    time_t  pvfs_fs_atimo;              /* attr timeout (sec) */
};

/*
 * Given fs, return driver private part.
 */
#define FS2PVFS(fs) \
    ((struct pvfs_filesystem *)(fs)->fs_private)

#define I2PI(ino)   ((struct pvfs_inode *)((ino)->i_private))

/*
 * pvfs file identifiers format.
 */
struct pvfs_inode_identifier {
    dev_t   dev;                    /* device number */
    ino_t   ino;                    /* i-number */
#ifdef HAVE_GENERATION
    unsigned int gen;               /* generation number */
#endif
};

/*
 * Driver-private i-node information
 */
struct pvfs_inode {
    unsigned
        pi_seekok       : 1,                    /* can seek? */
        pi_attrvalid    : 1,                    /* cached attrs ok? */
        pi_ref_set      : 1,                    /* cached attrs ok? */
        pi_resetfpos    : 1;                    /* reset fpos? */
    struct pvfs_inode_identifier pi_ident;      /* unique identifier */
    struct file_identifier pi_fileid;           /* ditto */
    int pi_fd;                                  /* host fildes */
    PVFS_object_ref pi_ref;                     /* pvfs file ref */
    int pi_oflags;                              /* flags, from open */
    unsigned pi_nopens;                         /* soft ref count */
    _SYSIO_OFF_T pi_fpos;                       /* current pos */
    time_t  pi_attrtim;                         /* attrs expire time */
    time_t  pi_handletim;                       /* handle expire time */
};

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

static inline int pvfs_attrs_valid(struct pvfs_inode * pino, time_t t)
{
    return (pino->pi_attrtim && (t < pino->pi_attrtim));
}

/*
 * stat the file remotely
 */
static int pvfs_stat(const char *path, struct inode *ino, time_t t, struct intnl_stat *buf)
{
    SYSIO_PVFS_FENTER();
    struct pvfs_inode * pino = NULL;
    int err = 0;
    struct intnl_stat stbuf;
    unsigned fhandle_set = 0;
    PVFS_credentials creds;
    PVFS_object_ref ref;
    PVFS_sysresp_getattr getattr_resp;
    PVFS_sysresp_lookup path_lookup_resp;
    PVFS_fs_id fsid;
    char fs_path[4096];

    /* get the pvfs creds */
    PVFS_util_gen_credentials(&creds);

    pino = ino ? I2PI(ino) : NULL;

    memset(fs_path, 0, 4096);

    /* get the fs ids*/

    /* if the full path is available, use the pvfs resolve path cmd */
    if(path)
    {
        char path_expand[4096];

        memset(&ref, 0, sizeof(ref));
        memset(path_expand, 0, 4096);

        /* if the path does not have the pvfs root, add it to the front of the path and resolve */
        if(strncmp(path, mps_root, strlen(mps_root)) != 0)
        {
            strcat(path_expand, mps_root);
        }
        strcat(path_expand, path);

        /* resolve the path and setup the reference */
        err = PVFS_util_resolve(path_expand, &fsid, fs_path, 4096);
        if(err)
        {
            SYSIO_PVFS_FEXIT();
            return -pvfs_error_to_sysio_error(err);
        }
        ref.fs_id = fsid;
        ref.handle = 0;
    }
    /* if the inode is valid, don't lookup */
    if(pino && pino->pi_ref_set)
    {
        memset(&getattr_resp, 0, sizeof(getattr_resp));
        err = PVFS_sys_getattr(pino->pi_ref, PVFS_ATTR_SYS_ALL, &creds, &getattr_resp, NULL);
        if(!err)
        {
            pvfs_attr_to_sysio_attr(&getattr_resp.attr, &pino->pi_ref, &stbuf);
            ino->i_stbuf = stbuf;
        }
        ref = pino->pi_ref;
    }
    /* if the path is set */
    else if (fs_path)
    {
        /* lookup the files in PVFS */
        memset(&path_lookup_resp, 0, sizeof(path_lookup_resp));
        err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds,
                            &path_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
        if(!err)
        {
            ref = path_lookup_resp.ref;
            memset(&getattr_resp, 0, sizeof(getattr_resp));
            err = PVFS_sys_getattr(path_lookup_resp.ref, PVFS_ATTR_SYS_ALL, &creds, &getattr_resp, NULL);
            if(!err)
            {
                pvfs_attr_to_sysio_attr(&getattr_resp.attr, &path_lookup_resp.ref, &stbuf);
            
                /* if the ino exists but we did not have the ref, update the inode state vars */
                if(pino)
                {
                    ino->i_stbuf = stbuf;
                }
            }
        }
    }
    else
    {
        SYSIO_PVFS_FABORT();
        abort();
    }

    /* if error */
    if (err) {
        /* invalidate attr time */
        if (pino)
        {
            pino->pi_attrtim = 0;
        }
        SYSIO_PVFS_FEXIT();

        return -pvfs_error_to_sysio_error(err);
    }

    /* set the attr time to now */
    if (pino) {
        pino->pi_attrtim = t;

        /* copy the attr if the buf is valid */
        if (buf)
            *buf = ino->i_stbuf;
        SYSIO_PVFS_FEXIT();
        return 0;
    }
    if (!buf)
    {
        SYSIO_PVFS_FEXIT();
        return 0;
    }

    /* copy attrs into buf */
    *buf = stbuf;
    SYSIO_PVFS_FEXIT();

    return 0;
}

/*
 * Create a new inode for the pvfs entry 
 */
static struct inode * pvfs_i_new(struct filesys *fs, time_t expiration, struct intnl_stat *buf)
{
    SYSIO_PVFS_FENTER();
    struct pvfs_inode * pino = NULL;
    struct inode * ino = NULL;

    pino = malloc(sizeof(struct pvfs_inode));
    if (!pino)
    {
        SYSIO_PVFS_FEXIT();
        return NULL;
    }
    memset(&pino->pi_ident, 0, sizeof(pino->pi_ident));
    pino->pi_seekok = 0;
    pino->pi_attrvalid = 0;
    pino->pi_resetfpos = 0;
    pino->pi_ident.dev = buf->st_dev;
    pino->pi_ident.ino = buf->st_ino;
#ifdef HAVE_GENERATION
    pino->pi_ident.gen = buf->st_gen;
#endif
    pino->pi_fileid.fid_data = &pino->pi_ident;
    pino->pi_fileid.fid_len = sizeof(pino->pi_ident);
    pino->pi_fd = -1;
    pino->pi_ref_set = 0;
    memset(&pino->pi_ref, 0, sizeof(pino->pi_ref));
    pino->pi_oflags = 0;
    pino->pi_nopens = 0;
    pino->pi_fpos = 0;
    pino->pi_attrtim = expiration;
    pino->pi_handletim = 0;
    ino = _sysio_i_new(fs, &pino->pi_fileid, buf, 0, &pvfs_i_ops, pino);
    if (!ino)
        free(pino);
    SYSIO_PVFS_FEXIT();
    return ino;
}

static int pvfs_fsswop_mount(const char *source,
            unsigned flags,
            const void *data,
            struct pnode *tocover,
            struct mount **mntp)
{
    SYSIO_PVFS_FENTER();
    int err = 0;
    struct nameidata nameidata;
    struct mount * mnt = NULL;
    struct pvfs_filesystem * pvfs_fs = NULL;
    struct inode *rootino;
    struct filesys * fs = NULL;
    unsigned long ul;
    char * opts = NULL;
    char * cp = NULL;
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
        SYSIO_PVFS_FEXIT();
        return -ENOENT;
    }

    opts = NULL;
    if (data && (len = strlen((char *)data))) {
        opts = malloc(len + 1);
        if (!opts)
        {
            SYSIO_PVFS_FEXIT();
            return -ENOMEM;
        }
        (void )strcpy(opts, data);
        if (_sysio_get_args(opts, v) - opts != (ssize_t )len)
        {
            SYSIO_PVFS_FEXIT();
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

    pvfs_fs = malloc(sizeof(struct pvfs_filesystem));
    if (!pvfs_fs) {
        err = -ENOMEM;
        goto error;
    }
    pvfs_fs->pvfs_fs_atimo = ul;
    if ((unsigned long)pvfs_fs->pvfs_fs_atimo != ul) {
        err = -EINVAL;
        goto error;
    }
    fs = _sysio_fs_new(&pvfs_inodesys_ops, 0, pvfs_fs);
    if (!fs) {
        err = -ENOMEM;
        goto error;
    }

    /*
     * Get root i-node.
     */
    t = _SYSIO_LOCAL_TIME();
    err = pvfs_stat("/", NULL, 0, &stbuf);
    if (err)
        goto error;
    rootino = pvfs_i_new(fs, t + FS2PVFS(fs)->pvfs_fs_atimo, &stbuf);
    if (!rootino) {
        err = -ENOMEM;
        goto error;
    }

    /* mount it */
    err = _sysio_mounti(fs, rootino, flags, tocover, &mnt);
    if (!err) {
        *mntp = mnt;
    }
    I_PUT(rootino);
    pvfs_internal_mount = mnt;

    SYSIO_PVFS_FEXIT();
    return err;

error:
    if (mnt) {
        if (_sysio_do_unmount(mnt) != 0)
        {
            SYSIO_PVFS_FABORT();
            abort();
        }
        rootino = NULL;
        fs = NULL;
        pvfs_fs = NULL;
    }
    if (rootino)
        I_RELE(rootino);
    if (fs) {
        FS_RELE(fs);
        pvfs_fs = NULL;
    }
    if (pvfs_fs)
        free(pvfs_fs);
    if (opts)
        free(opts);

    SYSIO_PVFS_FEXIT();
    return err;
}

int _pvfs_sysio_drv_init_all()
{
    SYSIO_PVFS_FENTER();
    extern int _sysio_native_init(void);

    /*
     * Only use the native sysio driver
     */
    int (*drvinits[])(void) = {
            NULL
    };

    int (**f)(void);
    int err = 0;

    err = 0;
    f = drvinits;
    while (*f) {
        err = (**f++)();
        if (err)
        {
            SYSIO_PVFS_FEXIT();
            return err;
        }
    }
    SYSIO_PVFS_FEXIT();
    return 0;
}

int _pvfs_sysio_init()
{
	int	err = 0;

    SYSIO_PVFS_FENTER();

    err = _sysio_fssw_register("pvfs", &pvfs_fssw_ops);

    SYSIO_PVFS_FEXIT();

	return err;
}

/*
 * invalidate an inode and erase the handle
 */
static int pvfs_i_invalid(struct inode *inop, struct intnl_stat *stat)
{
    struct pvfs_inode * pino = NULL;

    SYSIO_PVFS_FENTER();
    /*
     * Validate passed in inode against stat struct info
     */
    pino = I2PI(inop);

    if (!pino->pi_attrtim ||
        (pino->pi_ident.dev != stat->st_dev ||
         pino->pi_ident.ino != stat->st_ino ||
#ifdef HAVE_GENERATION
         pino->pi_ident.gen != stat->st_gen ||
#endif
         ((inop)->i_stbuf.st_mode & S_IFMT) != (stat->st_mode & S_IFMT)) ||
        (((inop)->i_stbuf.st_rdev != stat->st_rdev) &&
           (S_ISCHR((inop)->i_stbuf.st_mode) ||
            S_ISBLK((inop)->i_stbuf.st_mode)))) {
        pino->pi_attrtim = 0;           /* invalidate attrs */
        pino->pi_handletim = 0;         /* invalidate handle */
        pino->pi_ref_set = 0;
        memset(&pino->pi_ref, 0, sizeof(pino->pi_ref));
        SYSIO_PVFS_FEXIT();
        return 1;
    }
    SYSIO_PVFS_FEXIT();
    return 0;
}

static struct inode * pvfs_iget(struct filesys *fs, time_t expire, struct intnl_stat *stbp)
{
    struct inode * ino = NULL;
    struct pvfs_inode_identifier ident;
    struct file_identifier fileid;

    SYSIO_PVFS_FENTER();
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
        I2PI(ino)->pi_attrtim = expire;
        SYSIO_PVFS_FEXIT();
        return ino;
    }
    SYSIO_PVFS_FEXIT();
    return pvfs_i_new(fs, expire, stbp);
}

static int pvfs_ibind(struct filesys *fs, char *path, time_t t, struct inode **inop)
{
    struct intnl_stat ostbuf, stbuf;
    int err = 0;
    struct inode * ino = NULL;

    SYSIO_PVFS_FENTER();

    if (*inop)
        ostbuf = (*inop)->i_stbuf;

    err = pvfs_stat(path, *inop, t, &stbuf);
    if (err)
    {
        SYSIO_PVFS_FEXIT();
        return err;
    }

    /* 
     * Validate?
     */
    if (*inop) {
        if (!pvfs_i_invalid(*inop, &ostbuf))
        {
            SYSIO_PVFS_FEXIT();
            return 0;
        }
        /*
         * Invalidate.
         */
        _sysio_i_undead(*inop);
        *inop = NULL;
    }

    if (!(ino = pvfs_iget(fs, t + FS2PVFS(fs)->pvfs_fs_atimo, &stbuf)))
    {
        SYSIO_PVFS_FEXIT();
        return -ENOMEM;
    }

    *inop = ino;
    SYSIO_PVFS_FEXIT();
    return 0;
}

/*
 */
static int pvfs_inop_open(struct pnode *pno, int flags, mode_t mode)
{
    struct pvfs_inode * pino = NULL;
    char * path = NULL;
    struct inode * ino = NULL;
    int fd = 0;
    PVFS_object_ref file_ref;
    int file_exists = 0;
    int append_ii_fpos = 0;
    int err = 0;
    PVFS_object_ref ref;
    PVFS_sys_attr pattr;
    PVFS_credentials creds;
    PVFS_sysresp_create create_resp;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_fs_id fsid;
    char fs_path[4096];
    char * base_str = NULL;
    char * component_str = NULL;

    SYSIO_PVFS_FENTER();

    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_PVFS_FEXIT();
        return -ENOMEM;
    }

    if (flags & O_WRONLY) {
        /*
         * Promote write-only attempt to RW.
         */
        flags &= ~O_WRONLY;
        flags |= O_RDWR;
    }

    PVFS_util_gen_credentials(&creds);

    /* get the fs ids and paths*/
    err = PVFS_util_resolve(path, &fsid, fs_path, 4096);
    if(err)
    {
        SYSIO_PVFS_FINFO("could not resolve path... path = %s", path);
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    sysio_pvfs_split_path(fs_path, &base_str, &component_str);
    ref.fs_id = fsid;
    ref.handle = 0;

    /* get the file handle and set exist flag */
    if((err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds, &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL)) == 0)
    {
        memcpy(&file_ref, &lookup_resp.ref, sizeof(file_ref));
        file_exists = 1;
    }
        
    /*
     * create the file
     */
    if(flags & O_CREAT)
    {
        PVFS_sys_attr pattr;
        PVFS_sysresp_lookup lookup_response;
        PVFS_sysresp_create create_response;
 
        /* 
         * if we are creating a file, O_EXCL requires that the file does
         * does not exist... otherwise exit with error
         */ 
        if((flags & O_EXCL) == O_EXCL)
        {
            /* if the O_CREATE | O_EXCL flag is violated, exit */
            if(file_exists)
            {
                err = -pvfs_error_to_sysio_error(-PVFS_EEXIST);
                goto cleanup;
            }
        }

        /* erase existing attrs */
        memset(&pattr, 0, sizeof(pattr));

        /* set the file mode */ 
        pattr.owner = getuid();
        pattr.group = getgid();
        pattr.atime = time(NULL);
        pattr.mtime = pattr.atime;
        pattr.dfile_count = 0;
        pattr.perms = mode & 0777; /* this is the only user defined value */
        pattr.mask = PVFS_ATTR_SYS_ALL_SETABLE;

        /* erase responses */
        memset(&lookup_response, 0, sizeof(lookup_response));
        memset(&create_response, 0, sizeof(create_response));

        /* if the file does not exist, create it */
        if(!file_exists)
        {
            /* lookup the base file handle */
            err = PVFS_sys_lookup(ref.fs_id, base_str, &creds, &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);
            if(err)
            {
                    err = -pvfs_error_to_sysio_error(err);
                    goto cleanup;
            }

            /* create the file using the base handle and the component string path */
            err = PVFS_sys_create(component_str, lookup_resp.ref, pattr, &creds, NULL, &create_resp, NULL, NULL);
            if(err)
            {
                    err = -pvfs_error_to_sysio_error(err);
                    goto cleanup;
            }
            memcpy(&file_ref, &create_resp.ref, sizeof(file_ref));
            file_exists = 1;
        }
        /* else, do nothing */
    }

    /*
     * resize the file to zero bytes if it exists
     */
    if(flags & O_TRUNC)
    {
        /* If the file exists, get the file handle and resize to 0 bytes */
        if(file_exists)
        {
            PVFS_sysresp_lookup lookup_response;

            /* erase responses */
            memset(&lookup_response, 0, sizeof(lookup_response));

            /* lookup the base file handle */
            err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds, &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);
            if(err)
            {
                err = -pvfs_error_to_sysio_error(err);
                goto cleanup;
            }

            err = PVFS_sys_truncate(lookup_resp.ref, 0, &creds, NULL);
            if(err < 0)
            {
                err = -pvfs_error_to_sysio_error(err);
                goto cleanup;
            }
        }
        /* if the file does not exist, lookup the base handle and create the file using the component string */
        else
        {
            PVFS_sysresp_lookup lookup_response;
            PVFS_sysresp_create create_response;

            /* erase responses */
            memset(&lookup_response, 0, sizeof(lookup_response));
            memset(&create_response, 0, sizeof(create_response));

            /* lookup the base file handle */
            err = PVFS_sys_lookup(ref.fs_id, base_str, &creds, &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);
            if(err)
            {
                err = -pvfs_error_to_sysio_error(err);
                goto cleanup;
            }

            /* create the file using the base handle and the component string path */
            err = PVFS_sys_create(component_str, lookup_resp.ref, pattr, &creds, NULL, &create_resp, NULL, NULL);
            if(err)
            {
                err = -pvfs_error_to_sysio_error(err);
                goto cleanup;
            }
            memcpy(&file_ref, &create_resp.ref, sizeof(file_ref));
            file_exists = 1;
        }
    }

    /*
     * append to the file
     */
    if(flags & O_APPEND)
    {
        /* can't append to a file that does not exist */
        if(!file_exists)
        {
            err = -pvfs_error_to_sysio_error(PVFS_ENOENT);
            goto cleanup;
        }
        else
        {
            PVFS_sysresp_lookup lookup_response;
            PVFS_sysresp_getattr getattr_response;

            /* erase responses */
            memset(&getattr_response, 0, sizeof(getattr_response));
            memset(&lookup_response, 0, sizeof(lookup_response));

            err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds, &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW, NULL);
            if(err)
            {
                err = -pvfs_error_to_sysio_error(err);
                goto cleanup;
            }

            err = PVFS_sys_getattr(lookup_resp.ref, PVFS_ATTR_SYS_ALL_NOHINT, &creds, &getattr_response, NULL);
            if(err < 0)
            {
                err = -pvfs_error_to_sysio_error(err);
                goto cleanup;
            }
            append_ii_fpos = getattr_response.attr.size;
        }
    }

    /* if file still does not exist, exit */
    if(!file_exists) 
    {
        err = -pvfs_error_to_sysio_error(PVFS_ENOENT);
        goto cleanup;
    }

    /*
     */ 
    ino = pno->p_base->pb_ino;
    if (!ino && file_exists) {
        struct filesys * fs = NULL;

        /*
         * Success but we need to return an i-node.
         */
        fs = pno->p_mount->mnt_fs;
        err = pvfs_ibind(fs, path, _SYSIO_LOCAL_TIME() + FS2PVFS(fs)->pvfs_fs_atimo, &ino);
        if (err) {
            if (err == -EEXIST)
            {
                SYSIO_PVFS_FABORT();
                abort();
            }
            fd = err;
        }
    } 
    else
    {
        I_GET(ino);
    }

    if (fd < 0)
    {
        err = -errno;
        goto cleanup;
    }

    /*
     * Remember this new open.
     */
    pino = I2PI(ino);
    pino->pi_nopens++;
    assert(pino->pi_nopens);
    do {
        if (pino->pi_fd >= 0) {
            if ((pino->pi_oflags & O_RDWR) ||
                (flags & (O_RDONLY|O_WRONLY|O_RDWR)) == O_RDONLY) {
                break;
            }
        }
        /*
         * Invariant; First open. Must init.
         */
        pino->pi_resetfpos = 0;
        pino->pi_fpos = append_ii_fpos;
        pino->pi_fd = fd;
        pino->pi_ref_set = 1;
        memcpy(&pino->pi_ref, &file_ref, sizeof(pino->pi_ref));
        pino->pi_seekok = 1;
    } while (0);

    I_PUT(ino);

cleanup:
    if(path)
    {
        free(path);
    }
    if(base_str)
    {
        free(base_str);
    }
    if(component_str)
    {
        free(component_str);
    }

    SYSIO_PVFS_FEXIT();
	return err;
}

/*
 */
static int pvfs_inop_lookup(struct pnode *pno, struct inode **inop, struct intent *intnt, const char *path)
{
    int err = 0;
    time_t t;
    char * full_path = NULL;
    struct filesys *fs;
    struct pvfs_inode * pino;
    struct ionode * ino;

    SYSIO_PVFS_FENTER();

    t = _SYSIO_LOCAL_TIME();

    *inop = pno->p_base->pb_ino;

    if(pno)
    {
        char * fp = _sysio_pb_path(pno->p_base, '/');
        free(fp);
    }
    else
    {
        SYSIO_PVFS_FINFO("unkown lookup!");
    }

    /* use cached values */
    if(*inop && (path || !intnt || (intnt->int_opmask & INT_GETATTR) == 0) && pvfs_attrs_valid(I2PI(*inop), t))
    {   
        SYSIO_PVFS_FEXIT();
        return 0;
    }

    full_path = _sysio_pb_path(pno->p_base, '/');
    if(full_path)
    {
        fs = pno->p_mount->mnt_fs;
        err = pvfs_ibind(fs, full_path, t + FS2PVFS(fs)->pvfs_fs_atimo, inop);
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

    SYSIO_PVFS_FEXIT();
	return err;
}

/*
 */
static int pvfs_inop_getattr(struct pnode *pno, struct intnl_stat *stbuf)
{
    SYSIO_PVFS_FENTER();

    int err = 0;
    char * path = NULL;
    char fs_path[4096];
    char * path_base_str = NULL;
    char * path_component_str = NULL;
    PVFS_object_ref ref;
    PVFS_sysresp_getattr getattr_resp;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_credentials creds;

    /* erase PVFS data types */
    memset(fs_path, 0, 4096);
    memset(&ref, 0, sizeof(ref));
    memset(&creds, 0, sizeof(creds));
    memset(&lookup_resp, 0, sizeof(lookup_resp));
    memset(&getattr_resp, 0, sizeof(getattr_resp));

    /* get the path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_PVFS_FEXIT();
        return -ENOMEM;
    }

    /* get the fs id*/
    err = PVFS_util_resolve(path, &ref.fs_id, fs_path, 4096);
    if(err)
    {
        SYSIO_PVFS_FEXIT();
        return -pvfs_error_to_sysio_error(err);
    }
    ref.handle = 0;

    /* lookup the files in PVFS */
    err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds,
                       &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                        NULL);
    if(!err)
    {
        err = PVFS_sys_getattr(lookup_resp.ref, PVFS_ATTR_SYS_ALL, &creds, &getattr_resp, NULL);
        if(!err)
        {
            pvfs_attr_to_sysio_attr(&getattr_resp.attr, &lookup_resp.ref, stbuf);
        }
    }

    /* cleanup */
    if(path)
    {
        free(path);
    }

    SYSIO_PVFS_FEXIT();
	return -pvfs_error_to_sysio_error(err);
}

/*
 */
static int pvfs_inop_setattr(struct pnode *pno, unsigned mask, struct intnl_stat *stbuf)
{
    SYSIO_PVFS_FENTER();

    int err = 0;
    char * path = NULL;
    char fs_path[4096];
    char * path_base_str = NULL;
    char * path_component_str = NULL;
    PVFS_object_ref ref;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_credentials creds;
    PVFS_sys_attr pattr;

    /* erase PVFS data types */
    memset(fs_path, 0, 4096);
    memset(&ref, 0, sizeof(ref));
    memset(&creds, 0, sizeof(creds));
    memset(&lookup_resp, 0, sizeof(lookup_resp));

    /* get the path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_PVFS_FEXIT();
        return -ENOMEM;
    }

    PVFS_util_gen_credentials(&creds);

    /* get the fs id*/
    err = PVFS_util_resolve(path, &ref.fs_id, fs_path, 4096);
    if(err)
    {
        SYSIO_PVFS_FEXIT();
        return -pvfs_error_to_sysio_error(err);
    }
    ref.handle = 0;

    /* lookup the files in PVFS */
    err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds,
                       &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                        NULL);
    if(!err)
    {
        struct inode * ino = NULL;
        struct pvfs_inode * pino = NULL;

        memset(&pattr, 0, sizeof(pattr)); 
        //sysio_attr_to_pvfs_attr(stbuf, &pattr);

        if(mask & SETATTR_LEN)
        {

            err = PVFS_sys_truncate(lookup_resp.ref, stbuf->st_size, &creds, NULL);
            if(err < 0)
            {
                return -pvfs_error_to_sysio_error(err);
            }
        }
        else
        {
            /* convert the sysio mask to zoidfs */
            if(mask & SETATTR_MODE)
            {
                pattr.mask = pattr.mask | PVFS_ATTR_SYS_PERM;
                pattr.perms = stbuf->st_mode & 0777;
            }

            if(mask & SETATTR_UID)
            {
                pattr.mask = pattr.mask | PVFS_ATTR_SYS_UID;
                pattr.owner = stbuf->st_uid;
            }

            if(mask & SETATTR_GID)
            {
                pattr.mask = pattr.mask | PVFS_ATTR_SYS_GID;
                pattr.group = stbuf->st_gid;
            }

            if(mask & SETATTR_ATIME)
            {
                pattr.mask = pattr.mask | PVFS_ATTR_SYS_ATIME;
                pattr.atime = stbuf->st_atime;
            }
    
            if(mask & SETATTR_MTIME)
            {
                pattr.mask = pattr.mask | PVFS_ATTR_SYS_MTIME;
                pattr.mtime = stbuf->st_mtime;
            }
 
            err = PVFS_sys_setattr(lookup_resp.ref, pattr, &creds, NULL);
            if(err)
            {
                SYSIO_PVFS_FEXIT();
                return -pvfs_error_to_sysio_error(err);
            }
        }

        /* update the inode so that cached attrs are invalid */
        ino = pno->p_base->pb_ino;
        assert(ino);
        pino = I2PI(ino);
        pino->pi_attrvalid = 0;
        pino->pi_attrtim = 0;
    }

    /* cleanup */
    if(path)
    {
        free(path);
        path = NULL;
    }

    SYSIO_PVFS_FEXIT();
	return -pvfs_error_to_sysio_error(err);
}

/*
 */
static ssize_t pvfs_filldirentries(struct pnode *pno, _SYSIO_OFF_T *posp, char *buf, size_t nbytes)
{
    int err = 0;

    SYSIO_PVFS_FENTER();

    SYSIO_PVFS_FEXIT();
	return 0;
}

/*
 */
static int pvfs_inop_mkdir(struct pnode *pno, mode_t mode)
{
    SYSIO_PVFS_FENTER();

    char * path;
    int err;
    char fs_path[4096];
    char * path_base_str = NULL;
    char * path_component_str = NULL;
    PVFS_object_ref ref;
    PVFS_sysresp_mkdir mkdir_resp;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_credentials creds;
    PVFS_sys_attr pattr;

    /* erase PVFS data types */
    memset(fs_path, 0, 4096);
    memset(&ref, 0, sizeof(ref));
    memset(&creds, 0, sizeof(creds));
    memset(&pattr, 0, sizeof(pattr));
    memset(&lookup_resp, 0, sizeof(lookup_resp));
    memset(&mkdir_resp, 0, sizeof(mkdir_resp));

    /* set the attrs */
    pattr.owner = getuid();
    pattr.group = getgid();
    pattr.atime = time(NULL);
    pattr.mtime = pattr.atime;
    pattr.dfile_count = 0;
    pattr.perms = mode & 0777;
    pattr.mask = PVFS_ATTR_SYS_ALL_SETABLE;

    /* get the pvfs creds */
    PVFS_util_gen_credentials(&creds);

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

    /* get the fs id*/
    err = PVFS_util_resolve(path, &ref.fs_id, fs_path, 4096);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    ref.handle = 0;

    /* split the full path */
    sysio_pvfs_split_path(fs_path, &path_base_str, &path_component_str);

    /* lookup the file in PVFS */
    err = PVFS_sys_lookup(ref.fs_id, path_base_str, &creds,
                            &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

    err = PVFS_sys_mkdir(path_component_str, lookup_resp.ref, pattr, &creds,
                         &mkdir_resp, NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

cleanup:
    /* cleanup */
    if(path)
    {    
        free(path);
    }
    if(path_base_str)
    {
        free(path_base_str);
    }
    if(path_component_str)
    {
        free(path_component_str);
    }

    SYSIO_PVFS_FEXIT();
    return err;
}

/*
 */
static int pvfs_inop_rmdir(struct pnode *pno)
{
    SYSIO_PVFS_FENTER();

    char * path = NULL;
    int err;
    char fs_path[4096];
    char * path_base_str = NULL;
    char * path_component_str = NULL;
    PVFS_object_ref ref;
    PVFS_credentials creds;
    PVFS_sysresp_lookup lookup_resp;

    /* erase PVFS data types */
    memset(fs_path, 0, 4096);
    memset(&ref, 0, sizeof(ref));
    memset(&creds, 0, sizeof(creds));
    memset(&lookup_resp, 0, sizeof(lookup_resp));

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

    PVFS_util_gen_credentials(&creds);

    /* get the fs id*/
    err = PVFS_util_resolve(path, &ref.fs_id, fs_path, 4096);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    ref.handle = 0;

    /* split the full path */
    sysio_pvfs_split_path(fs_path, &path_base_str, &path_component_str);

    /* lookup the file in PVFS */
    err = PVFS_sys_lookup(ref.fs_id, path_base_str, &creds,
                            &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

    /* remove it */
    err = PVFS_sys_remove(path_component_str, lookup_resp.ref, &creds, NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

cleanup:
 
    /* cleanup */
    if(path)
    { 
        free(path);
    }
    if(path_base_str)
    {
        free(path_base_str);
    }
    if(path_component_str)
    {
        free(path_component_str);
    }

    SYSIO_PVFS_FEXIT();
	return err; 
}

/*
 */
static int pvfs_inop_link(struct pnode *old, struct pnode *new)
{
    SYSIO_PVFS_FENTER();

    /* PVFS2 does not support hard links */

    SYSIO_PVFS_FEXIT();
	return -EPERM;
}

/*
 */
static int pvfs_inop_readlink(struct pnode *pno, char *buf, size_t bufsiz)
{
    SYSIO_PVFS_FENTER();

    uint32_t mask;
    PVFS_object_ref ref;
    PVFS_credentials creds;
    PVFS_sysresp_getattr getattr_resp;
    char * path;
    int err;

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        SYSIO_PVFS_FEXIT();
        return -ENOMEM;
    }

    PVFS_util_gen_credentials(&creds);

    /* all attrs */
    mask = 0;

    /* readlink, get the attrs and copy the link target to the buffer */
    err = PVFS_sys_getattr(ref, mask, &creds, &getattr_resp, NULL);
    if(err < 0)
    {
        return pvfs_error_to_sysio_error(err);
    }

    strncpy(buf, getattr_resp.attr.link_target, bufsiz);

    /* cleanup */
    free(path);

    SYSIO_PVFS_FEXIT();
	return bufsiz;
}

/*
 */
static int pvfs_inop_close(struct pnode *pno)
{
    SYSIO_PVFS_FENTER();

    struct inode *ino = NULL;
    struct pvfs_inode *pino = NULL;
    int err = 0;
    char * path = NULL;

    /* get the full path */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

    ino = pno->p_base->pb_ino;
    assert(ino);
    pino = I2PI(ino);
    if (pino->pi_fd < 0)
    {
        SYSIO_PVFS_FABORT();
        abort();
    }

    assert(pino->pi_nopens);
    if (--pino->pi_nopens) 
    {
        err = 0;
        goto cleanup;
    }

    pino->pi_fd = -1;
    pino->pi_ref_set = 0;
    memset(&pino->pi_ref, 0, sizeof(pino->pi_ref));
    pino->pi_resetfpos = 0;
    pino->pi_fpos = 0;

cleanup:

    /* cleanup */
    if(path)
    {
        free(path);
    }

    SYSIO_PVFS_FEXIT();
	return err;
}

/*
 */
static int pvfs_inop_symlink(struct pnode *pno, const char *data)
{
    SYSIO_PVFS_FENTER();

    int err = 0;
    char * opath = NULL, * npath = NULL;
    PVFS_sys_attr pattr;
    PVFS_credentials creds;
    PVFS_object_ref opath_ref;
    PVFS_fs_id opath_fsid, npath_fsid;
    PVFS_sysresp_lookup opath_lookup_resp;
    PVFS_sysresp_symlink symlink_resp;
    char * opath_base_str = NULL;
    char * opath_component_str = NULL;
    char opath_fs_path[4096];

    /* erase PVFS data types */
    memset(opath_fs_path, 0, 4096);
    memset(&creds, 0, sizeof(creds));
    memset(&opath_lookup_resp, 0, sizeof(opath_lookup_resp));
    memset(&symlink_resp, 0, sizeof(symlink_resp));

    /* get the pvfs creds */
    PVFS_util_gen_credentials(&creds);

    /* get the sysio full paths */
    opath = _sysio_pb_path(pno->p_base, '/');
    npath = (char *)data;
    if (!(opath && npath)) {
        err = -ENOMEM;
        goto cleanup;
    }

    /* erase existing attrs */
    memset(&pattr, 0, sizeof(pattr));

    /* set the default link mode */
    pattr.owner = getuid();
    pattr.group = getgid();
    pattr.atime = time(NULL);
    pattr.mtime = pattr.atime;
    pattr.dfile_count = 0;
    pattr.perms = 0777;
    pattr.mask = PVFS_ATTR_SYS_ALL_SETABLE;

    /* get the fs ids*/
    err = PVFS_util_resolve(opath, &opath_fsid, opath_fs_path, 4096);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    opath_ref.fs_id = opath_fsid;
    opath_ref.handle = 0;

    /* split the full paths */
    sysio_pvfs_split_path(opath_fs_path, &opath_base_str, &opath_component_str);

    /* lookup the files in PVFS */
    err = PVFS_sys_lookup(opath_ref.fs_id, opath_base_str, &creds,
                            &opath_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

    /* symlink the files */
    err = PVFS_sys_symlink((char *)opath_component_str,
                               opath_lookup_resp.ref, (char *)npath,
                               pattr, &creds, &symlink_resp, NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

    /* cleanup */
cleanup:
    if(opath)
    {
        free(opath);
    }
    if(opath_component_str)
    {
        free(opath_component_str);
    }
    if(opath_base_str)
    {
        free(opath_base_str);
    }

    SYSIO_PVFS_FEXIT();
	return err;
}

/*
 */
static int pvfs_inop_unlink(struct pnode *pno)
{
    SYSIO_PVFS_FENTER();
    char * path = NULL;
    int err = 0;
    PVFS_credentials creds;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_object_ref ref;
    PVFS_fs_id fsid;
    char fs_path[4096];
    char * path_base_str = NULL;
    char * path_component_str = NULL;

    /* get the creds */
    PVFS_util_gen_credentials(&creds);

    /* get the path from sysio */
    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

    /* get the fs id*/
    err = PVFS_util_resolve(path, &fsid, fs_path, 4096);
    if(err)
    {
        goto cleanup;
    }
    ref.fs_id = fsid;
    ref.handle = 0;

    /* split the full path */
    sysio_pvfs_split_path(fs_path, &path_base_str, &path_component_str);

    /* lookup the file in PVFS */
    err = PVFS_sys_lookup(fsid, path_base_str, &creds,
                            &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        goto cleanup;
    }

    /* remove it */
    err = PVFS_sys_remove(path_component_str, lookup_resp.ref, &creds, NULL); 
    if(err)
    {
        goto cleanup;
    }

cleanup:
    if(path)
    {
        free(path);
    }
    
    if(path_base_str)
    {
        free(path_base_str);
    }

    if(path_component_str)
    {
        free(path_component_str);
    }

    SYSIO_PVFS_FEXIT();
    return -pvfs_error_to_sysio_error(err);
}

/*
 */
static int pvfs_inop_rename(struct pnode *old, struct pnode *new)
{
    SYSIO_PVFS_FENTER();

    int err;
    char * opath = NULL, * npath= NULL;
    PVFS_credentials creds;
    PVFS_object_ref npath_ref, opath_ref;
    PVFS_fs_id opath_fsid, npath_fsid;
    PVFS_sysresp_lookup npath_lookup_resp, opath_lookup_resp;
    char * opath_base_str = NULL;
    char * opath_component_str = NULL;
    char * npath_base_str = NULL;
    char * npath_component_str = NULL;
    char opath_fs_path[4096];
    char npath_fs_path[4096];

    /* get the pvfs creds */
    PVFS_util_gen_credentials(&creds);

    /* get the sysio full paths */
    opath = _sysio_pb_path(old->p_base, '/');
    npath = _sysio_pb_path(new->p_base, '/');
    if (!(opath && npath)) {
        err = -ENOMEM;
        goto out;
    }

    /* get the fs ids*/
    err = PVFS_util_resolve(opath, &opath_fsid, opath_fs_path, 4096);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto out;
    }
    opath_ref.fs_id = opath_fsid;
    opath_ref.handle = 0;

    err = PVFS_util_resolve(npath, &npath_fsid, npath_fs_path, 4096);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto out;
    }
    npath_ref.fs_id = npath_fsid;
    npath_ref.handle = 0;

    /* split the full paths */
    sysio_pvfs_split_path(opath_fs_path, &opath_base_str, &opath_component_str);
    sysio_pvfs_split_path(npath_fs_path, &npath_base_str, &npath_component_str);

    /* lookup the files in PVFS */
    err = PVFS_sys_lookup(opath_ref.fs_id, opath_base_str, &creds,
                            &opath_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto out;
    }

    err = PVFS_sys_lookup(npath_ref.fs_id, npath_base_str, &creds,
                            &npath_lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto out;
    }

    /* rename the file */
    err = PVFS_sys_rename((char *)opath_component_str, opath_lookup_resp.ref,
                            (char *)npath_component_str, npath_lookup_resp.ref,
                            &creds, NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto out;
    }

out:
    if (opath)
        free(opath);
    if (npath)
        free(npath);
    if(opath_component_str)
        free(opath_component_str);
    if(npath_component_str)
        free(npath_component_str);
    if(opath_base_str)
        free(opath_base_str);
    if(npath_base_str)
        free(npath_base_str);

    SYSIO_PVFS_FEXIT();
	return err;
}

static int pvfs_inop_read(struct ioctx *ioctx)
{
    SYSIO_PVFS_FENTER();

    int err = 0;
    char * path = NULL;
    char fs_path[4096];
    char * base_str = NULL;
    char * component_str = NULL;
    uint32_t * mem_sizes = NULL;
    void ** mem_starts = NULL;
    size_t mem_count = 0;
    uint32_t * file_sizes = NULL;
    uint64_t * file_starts = NULL;
    size_t file_count = 0;
    int i = 0;

    void * buffer = NULL;
    uint64_t offset = 0;
    uint64_t t_file_size = 0;

    PVFS_Request mem_req;
    PVFS_Request file_req;
    PVFS_size * displacements = NULL;
    PVFS_credentials creds;
    PVFS_sysresp_io io_resp;
    PVFS_sysresp_getattr getattr_resp;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_object_ref ref;

    struct inode *ino;
    struct pvfs_inode *pino;

    PVFS_util_gen_credentials(&creds);

    ino = ioctx->ioctx_pno->p_base->pb_ino;
    pino = I2PI(ino);

    /* get the full path of the file from the sysio pnode */
    path = _sysio_pb_path(ioctx->ioctx_pno->p_base, '/');
    if(!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

    err = PVFS_util_resolve(path, &ref.fs_id, fs_path, 4096);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    ref.handle = 0;

    /* split the full paths */
    sysio_pvfs_split_path(fs_path, &base_str, &component_str);

    err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds,
                            &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

    err = PVFS_sys_getattr(pino->pi_ref, PVFS_ATTR_SYS_ALL, &creds, &getattr_resp, NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    t_file_size = getattr_resp.attr.size;

    /* convert the iov data structures to pvfs data structures */
    mem_sizes = (uint32_t *)malloc(sizeof(uint32_t) * ioctx->ioctx_iovlen);
    mem_starts = (void *)malloc(sizeof(void *) * ioctx->ioctx_iovlen);
    file_sizes = (uint32_t *)malloc(sizeof(uint32_t) * ioctx->ioctx_xtvlen);
    file_starts = (uint64_t *)malloc(sizeof(uint64_t) * ioctx->ioctx_xtvlen);

    /* iovec data conversions */
    for(i = 0 ; i < ioctx->ioctx_iovlen ; i+=1)
    {
        mem_sizes[i] = (uint32_t)ioctx->ioctx_iov[i].iov_len;
        mem_starts[i] = (void *)ioctx->ioctx_iov[i].iov_base;
    }
    mem_count = i;

    if(mem_count > 1)
    {
        int j = 0;
        displacements = malloc(sizeof(PVFS_size) * mem_count);
        for(j = 0 ; j < mem_count ; j++)
        {
            displacements[j] = (intptr_t)mem_starts[j];
        }
        
        err = PVFS_Request_indexed(mem_count, mem_sizes, displacements, PVFS_BYTE, &mem_req);
        if(err < 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
        buffer = PVFS_BOTTOM;
    }
    else if(mem_count == 1)
    {
        err = PVFS_Request_contiguous(mem_sizes[0], PVFS_BYTE, &mem_req);
        if(err < 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
        buffer = mem_starts[0];
    }

    /* xtvec data conversions */
    uint64_t max_pos = 0; 
    for(i = 0 ; i < ioctx->ioctx_xtvlen ; i+=1)
    {
        file_starts[i] = (uint64_t)ioctx->ioctx_xtv[i].xtv_off + pino->pi_fpos;
        file_sizes[i] = (uint32_t)ioctx->ioctx_xtv[i].xtv_len;

        if(file_starts[i] + file_sizes[i] > t_file_size)
        {
            if(t_file_size >= file_starts[i])
            {
                file_sizes[i] = t_file_size - file_starts[i];
            }
            else
            {
                file_sizes[i] = 0;
            }
        }

        if(file_starts[i] + file_sizes[i] > max_pos)
        {
            max_pos = file_starts[i] + file_sizes[i];
        }
    }
    file_count = i;

    if(file_count > 1)
    {
        err = PVFS_Request_indexed(file_count, file_sizes, file_starts, PVFS_BYTE, &file_req);
        if(err < 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
    }
    else if(file_count == 1)
    {
        offset = file_starts[0];
        err = PVFS_Request_contiguous(file_sizes[0], PVFS_BYTE, &file_req);
        if(err < 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
    }

    err = PVFS_sys_io(lookup_resp.ref, file_req, offset, (void *)buffer, mem_req, &creds, &io_resp, PVFS_IO_READ, NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    
    size_t total = io_resp.total_completed;
    unsigned int index = 0;

    while(index < file_count && total >= file_sizes[index])
    {
        total -= file_sizes[index];
        ++index;
    }

    while(index < file_count)
    {
        file_sizes[index] = 0;
        ++index;
    }

    pino->pi_fpos = max_pos;

cleanup:
    /* cleanup */
    PVFS_Request_free(&mem_req);
    PVFS_Request_free(&file_req);

    if(mem_starts)
    {
        free(mem_starts);
    }
    if(mem_sizes)
    {
        free(mem_sizes);
    }
    if(file_starts)
    {
        free(file_starts);
    }
    if(file_sizes)
    {
        free(file_sizes);
    }
    if(path)
    {
        free(path);
    }
    if(base_str)
    {
        free(base_str);
    }
    if(component_str)
    {
        free(component_str);
    }
    if(displacements)
    {
        free(displacements);
    }

    SYSIO_PVFS_FEXIT();
	return err;
}

/*
 */
static int pvfs_inop_write(struct ioctx *ioctx)
{
    SYSIO_PVFS_FENTER();

    int err = 0;
    char * path = NULL;
    char fs_path[4096];
    char * base_str = NULL;
    char * component_str = NULL;
    uint32_t * mem_sizes = NULL;
    void ** mem_starts = NULL;
    uint32_t * file_sizes = NULL;
    uint64_t * file_starts = NULL;
    int i = 0;

    void * buffer = NULL;
    uint64_t offset = 0;

    PVFS_Request mem_req;
    PVFS_Request file_req;
    PVFS_size * displacements = NULL;
    PVFS_credentials creds;
    PVFS_sysresp_io io_resp;
    PVFS_sysresp_lookup lookup_resp;
    PVFS_object_ref ref;

    struct inode *ino;
    struct pvfs_inode *pino;

    int mem_count = 0;
    int file_count = 0;

    PVFS_util_gen_credentials(&creds);

    ino = ioctx->ioctx_pno->p_base->pb_ino;
    pino = I2PI(ino);

    /* get the full path of the file from the sysio pnode */
    path = _sysio_pb_path(ioctx->ioctx_pno->p_base, '/');
    if(!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

    err = PVFS_util_resolve(path, &ref.fs_id, fs_path, 4096);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }
    ref.handle = 0;

    /* split the full paths */
    sysio_pvfs_split_path(fs_path, &base_str, &component_str);

    err = PVFS_sys_lookup(ref.fs_id, fs_path, &creds,
                            &lookup_resp, PVFS2_LOOKUP_LINK_NO_FOLLOW,
                            NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

    /* do the data structure conversion */
    mem_sizes = (uint32_t *)malloc(sizeof(uint32_t) * ioctx->ioctx_iovlen);
    mem_starts = (void *)malloc(sizeof(void *) * ioctx->ioctx_iovlen);
    file_sizes = (uint32_t *)malloc(sizeof(uint32_t) * ioctx->ioctx_xtvlen);
    file_starts = (uint64_t *)malloc(sizeof(uint64_t) * ioctx->ioctx_xtvlen);

    /* iovec data conversions */
    for(i = 0 ; i < ioctx->ioctx_iovlen ; i+=1)
    {
        mem_sizes[i] = (uint32_t)ioctx->ioctx_iov[i].iov_len;
        mem_starts[i] = (void *)ioctx->ioctx_iov[i].iov_base;
    }
    mem_count = i;

    if(mem_count > 1)
    {
        int j = 0;
        displacements = malloc(sizeof(PVFS_size) * mem_count);
        for(j = 0 ; j < mem_count ; j++)
        {
            displacements[j] = (intptr_t)mem_starts[j];
        }
        
        err = PVFS_Request_indexed(mem_count, mem_sizes, displacements, PVFS_BYTE, &mem_req);
        if(err != 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
        buffer = PVFS_BOTTOM;
    }
    else if(mem_count == 1)
    {
        err = PVFS_Request_contiguous(mem_sizes[0], PVFS_BYTE, &mem_req);
        if(err != 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
        buffer = mem_starts[0];
    }

    /* xtvec data conversions */
    uint64_t max_pos = 0; 
    for(i = 0 ; i < ioctx->ioctx_xtvlen ; i+=1)
    {
        file_starts[i] = (uint64_t)ioctx->ioctx_xtv[i].xtv_off + pino->pi_fpos;
        file_sizes[i] = (uint32_t)ioctx->ioctx_xtv[i].xtv_len;
        if(file_starts[i] + file_sizes[i] > max_pos)
        {
            max_pos = file_starts[i] + file_sizes[i];
        }
    }
    file_count = i;

    if(file_count > 1)
    {
        err = PVFS_Request_indexed(file_count, file_sizes, file_starts, PVFS_BYTE, &file_req);
        if(err != 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
    }
    else if(file_count == 1)
    {
        offset = file_starts[0];
        err = PVFS_Request_contiguous(file_sizes[0], PVFS_BYTE, &file_req);
        if(err != 0)
        {
            err = -pvfs_error_to_sysio_error(err);
            goto cleanup;
        }
    }

    err = PVFS_sys_io(lookup_resp.ref, file_req, offset, (void *)buffer, mem_req, &creds, &io_resp, PVFS_IO_WRITE, NULL);
    if(err)
    {
        err = -pvfs_error_to_sysio_error(err);
        goto cleanup;
    }

    pino->pi_fpos = max_pos;

    /* cleanup */
cleanup:
    PVFS_Request_free(&mem_req);
    PVFS_Request_free(&file_req);

    if(mem_starts)
    {
        free(mem_starts);
    }
    if(mem_sizes)
    {
        free(mem_sizes);
    }
    if(file_starts)
    {
        free(file_starts);
    }
    if(file_sizes)
    {
        free(file_sizes);
    }
    if(path)
    {
        free(path);
    }
    if(base_str)
    {
        free(base_str);
    }
    if(component_str)
    {
        free(component_str);
    }
    if(displacements)
    {
        free(displacements);
    }

    SYSIO_PVFS_FEXIT();
	return err;
}

static _SYSIO_OFF_T pvfs_inop_pos(struct pnode *pno, _SYSIO_OFF_T off)
{
    SYSIO_PVFS_FENTER();
    SYSIO_PVFS_FEXIT();
	return 0;
}

/*
 */
static int pvfs_inop_iodone(struct ioctx *ioctx)
{
    SYSIO_PVFS_FENTER();
    SYSIO_PVFS_FEXIT();
	return 1;
}

/*
 */
static int pvfs_inop_fcntl(struct pnode *pno, int cmd, va_list ap, int *rtn)
{
    SYSIO_PVFS_FENTER();
    SYSIO_PVFS_FEXIT();
	return 0;
}

static int pvfs_inop_sync(struct pnode *pno)
{
    SYSIO_PVFS_FENTER();
    int err = 0;
    char * path = NULL;

    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

cleanup:

    if(path)
    {
        free(path);
    }

    SYSIO_PVFS_FEXIT();
	return err;
}

static int pvfs_inop_datasync(struct pnode *pno)
{
    SYSIO_PVFS_FENTER();
    int err = 0;
    char * path = NULL;

    path = _sysio_pb_path(pno->p_base, '/');
    if (!path)
    {
        err = -ENOMEM;
        goto cleanup;
    }

cleanup:
    if(path)
    {
        free(path);
    }

    SYSIO_PVFS_FEXIT();
	return err;
}

/*
 */
static int pvfs_inop_ioctl(struct pnode *pno, unsigned long int request, va_list ap)
{
    SYSIO_PVFS_FENTER();
    SYSIO_PVFS_FEXIT();
	return ENOTTY;
}

static int pvfs_inop_mknod(struct pnode *pno, mode_t mode, dev_t dev)
{
    SYSIO_PVFS_FENTER();
    SYSIO_PVFS_FEXIT();
	return ENOSYS;
}

#ifdef _HAVE_STATVFS
/* not supported */
static int pvfs_inop_statvfs(struct pnode *pno, struct intnl_statvfs *buf)
{
    SYSIO_PVFS_FENTER();
    SYSIO_PVFS_FEXIT();
	return ENOSYS;
}

#endif

/*
 * cleanup when the inop is gone
 */
static void pvfs_inop_gone(struct inode *ino)
{
    SYSIO_PVFS_FENTER();

    /* sysio cleanup only */
    free(ino->i_private);

    SYSIO_PVFS_FEXIT();
	return;
}

/*
 * cleanup when the fsop is gone
 */
static void pvfs_fsop_gone(struct filesys * fs)
{
    SYSIO_PVFS_FENTER();

    /* sysio cleanup only */
    free(fs->fs_private);

    SYSIO_PVFS_FEXIT();
}
