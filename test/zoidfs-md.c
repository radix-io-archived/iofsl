/*
 * zoidfs-md.c
 * Test the ZOIDFS metadata interface.
 */

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

#include "zoidfs.h"
#include "zoidfs-proto.h"
#include "iofwd_config.h"

#define NAMESIZE 255

int print_zoidfs_attr_t(zoidfs_attr_t *);

int main(int argc, char **argv) {
    int ret, created;
    uint32_t flags = 0;
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_attr_t resattr;
    size_t entry_count = 32;
    zoidfs_dirent_t *entries;
    char buffer[ZOIDFS_PATH_MAX];
    zoidfs_handle_t basedir_handle;
    zoidfs_cache_hint_t parent_hint;
    zoidfs_handle_t fhandle, dhandle;
    zoidfs_dirent_cookie_t cookie = 0;
    char new_fullpath_filename[NAMESIZE];
    char symlink[NAMESIZE], symlink_target[NAMESIZE];
    char link[NAMESIZE], link_target[NAMESIZE];
    char fullpath_dirname[NAMESIZE], component_dirname[NAMESIZE];
    char fullpath_filename[NAMESIZE], component_filename[NAMESIZE];

    if(argc < 2) {
        fprintf(stderr, "Usage: test-zoidfs <pvfs2 mount point>\n");
        exit(1);
    }

    snprintf(symlink, NAMESIZE, "%s/symlink", argv[1]);
    snprintf(symlink_target, NAMESIZE, "%s/test-zoidfs-fullpath", argv[1]);
    snprintf(link, NAMESIZE, "%s/link", argv[1]);
    snprintf(link_target, NAMESIZE, "%s/test-zoidfs-fullpath", argv[1]);
    snprintf(fullpath_dirname, NAMESIZE, "%s/test-dir-fullpath", argv[1]);
    snprintf(component_dirname, NAMESIZE, "test-dir-component");

    snprintf(fullpath_filename, NAMESIZE, "%s/test-zoidfs-fullpath", argv[1]);
    snprintf(new_fullpath_filename, NAMESIZE, "%s/new-test-zoidfs-fullpath",
             argv[1]);
    snprintf(component_filename, NAMESIZE, "test-zoidfs-component");

    ret = zoidfs_init();
    if(ret != ZFS_OK) {
        return ret;
    }

    /* noop */
    zoidfs_null();

    ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    if(ret == ZFS_OK) {
        /* If it exists, we delete it */
        ret = zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);
        if(ret != ZFS_OK) {
            goto exit;
        }
    }

    ret = zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    if(ret == ZFS_OK) {
        ret = zoidfs_remove(NULL, NULL, fullpath_filename, NULL);
        if(ret != ZFS_OK) {
            goto exit;
        }
    }

    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0755;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* Try create with handle and component */
    ret = zoidfs_create(&basedir_handle, component_filename, NULL, &sattr,
                        &fhandle, &created);
    if(ret != ZFS_OK) {
        fprintf(stderr, "comp create failed!\n");
        goto exit;
    }

    /* Check that created == 1 */
    assert(created);
    ret = zoidfs_create(&basedir_handle, component_filename, NULL, &sattr,
                        &fhandle, &created);
    if(ret != ZFS_OK) {
        fprintf(stderr, "comp recreate failed!\n");
        goto exit;
    }

    /* Check that created == 0 */
    assert(created == 0);
    ret = zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &fhandle,
                        &created);
    if(ret != ZFS_OK) {
        fprintf(stderr, "fp create failed!\n");
        goto exit;
    }

    assert(created);

    ret = zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &fhandle,
                        &created);
    if(ret != ZFS_OK) {
        goto exit;
    }

    assert(created == 0);

    /* Try lookup with handle and component */
    ret = zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Try lookup with full_path */
    ret = zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Try lookup with full_path */
    ret = zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Try lookup with full_path */
    ret = zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    sattr.mask = ZOIDFS_ATTR_MODE;
    sattr.mode = 0666;

    /* Set some attributes */
    ret = zoidfs_setattr(&fhandle, &sattr, &resattr);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Verify attribs were set correctly */
    assert(resattr.mode == 0666);

    /* Resize */
    ret = zoidfs_resize(&fhandle, 1000);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Now check the size */
    ret = zoidfs_getattr(&fhandle, &resattr);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Correct size? */
    assert(resattr.mask & ZOIDFS_ATTR_SIZE && resattr.size == 1000);

    /* Rename the files */
    ret = zoidfs_rename(&basedir_handle, component_filename, NULL, NULL, NULL,
                        new_fullpath_filename, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_rename(NULL, NULL, new_fullpath_filename, &basedir_handle,
                        component_filename, NULL, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* create symlinks */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    ret = zoidfs_symlink(NULL, NULL, symlink, NULL, NULL, symlink_target,
                         &sattr, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

#ifndef HAVE_DISPATCHER_LIBSYSIO
    /* create a link */
    ret = zoidfs_link(NULL, NULL, link, NULL, NULL, link_target,
                         NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }
#endif
    
    /* readlink */
    ret = zoidfs_lookup(NULL, NULL, symlink, &fhandle);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_readlink(&fhandle, buffer, ZOIDFS_PATH_MAX);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_remove(NULL, NULL, symlink, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Remove the files */
    ret = zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_rename(NULL, NULL, fullpath_filename, NULL, NULL,
                        new_fullpath_filename, NULL, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_remove(NULL, NULL, new_fullpath_filename, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Directory operations */
    sattr.mask = ZOIDFS_ATTR_ALL;
    ret = zoidfs_lookup(&basedir_handle, component_dirname, NULL, &dhandle);
    if(ret == ZFS_OK) {
        /* If it exists, we delete it */
        ret = zoidfs_remove(&basedir_handle, component_dirname, NULL, NULL);
        if(ret != ZFS_OK) {
            goto exit;
        }
    }

    ret = zoidfs_lookup(NULL, NULL, fullpath_dirname, &dhandle);
    if(ret == ZFS_OK) {
        ret = zoidfs_remove(NULL, NULL, fullpath_dirname, NULL);
        if(ret != ZFS_OK) {
            goto exit;
        }
    }

    /* Try mkdir with handle and component */
    ret = zoidfs_mkdir(&basedir_handle, component_dirname, NULL, &sattr,
                       &parent_hint);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Try mkdir with fullpath */
    ret = zoidfs_mkdir(NULL, NULL, fullpath_dirname, &sattr, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

#ifndef HAVE_DISPATCHER_LIBSYSIO
    /* List dirents */
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    if (!entries) {
        perror("zoidfs-md: malloc() failed");
        goto exit;
    }
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    ret = zoidfs_readdir(&basedir_handle, cookie, &entry_count, entries, flags,
                         NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }
    free(entries);
#endif

    /* Cleanup directories */
    ret = zoidfs_remove(&basedir_handle, component_dirname, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }
    ret = zoidfs_remove(NULL, NULL, fullpath_dirname, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

exit:

    zoidfs_finalize();

    if(ret != ZFS_OK) {
        fprintf(stderr, "Test failed: error: %d\n", ret);
    } else {
        fprintf(stdout, "Test succeeded.\n");
    }

    return ret;
}


int print_zoidfs_attr_t(zoidfs_attr_t *zattr) {
    printf("%d %u %u %u %u %u %lu %u %u %lu %lu %u %lu %u %lu %u\n",
            zattr->type, zattr->mask, zattr->mode, zattr->nlink, zattr->uid,
            zattr->gid, zattr->size, zattr->blocksize, zattr->fsid,
            zattr->fileid, zattr->atime.seconds, zattr->atime.nseconds,
            zattr->mtime.seconds, zattr->mtime.nseconds, zattr->ctime.seconds,
            zattr->ctime.nseconds);

    return 0;
}

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
