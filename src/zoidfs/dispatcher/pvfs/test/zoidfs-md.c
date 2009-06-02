
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include "zoidfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define BUFSIZE 255
int zoid_init(int);

int main(int argc, char * argv[])
{
    int ret;
    int created;
    struct timeval now;
    uint32_t flags = 0;
    zoidfs_handle_t basedir_handle;
    zoidfs_handle_t res_handle;
    zoidfs_sattr_t sattr, dsattr;
    zoidfs_attr_t resattr;
    char  fullpath_filename[BUFSIZE];
    char  component_filename[BUFSIZE];
    char  new_fullpath_filename[BUFSIZE];
    char  symlink[BUFSIZE], symlink_target[BUFSIZE], buffer[BUFSIZE];
 
    size_t entry_count = 32;
    zoidfs_dirent_t *entries;
    zoidfs_dirent_cookie_t cookie = 0;

    if(argc < 2)
    {
        fprintf(stderr, "usage: test-zoidfs <pvfs2 mount point>\n");
        exit(1);
    }

    if(zoid_init(0))
    {
        return 1;
    }

    sprintf(symlink, "%s/symlink", argv[1]);
    sprintf(symlink_target, "%s/test-zoidfs-fullpath", argv[1]);
    sprintf(fullpath_filename, "%s/test-zoidfs-fullpath", argv[1]);
    sprintf(new_fullpath_filename, "%s/new-test-zoidfs-fullpath", argv[1]);
    sprintf(component_filename, "test-zoidfs-component");

    ret = zoidfs_init();
    if(ret != ZFS_OK)
    {
        return ret;
    }

    ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_lookup(&basedir_handle, component_filename, NULL, &res_handle);
    if(ret == ZFS_OK)
    {
        /* if it exists we delete it */
        ret = zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);
        if(ret != ZFS_OK)
        {
            goto exit;
        }
    }

    ret = zoidfs_lookup(NULL, NULL, fullpath_filename, &res_handle);
    if(ret == ZFS_OK)
    {
        ret = zoidfs_remove(NULL, NULL, fullpath_filename, NULL);
        if(ret != ZFS_OK)
        {
            goto exit;
        }
    }

    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0600;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* try create with handle and component */
    ret = zoidfs_create(&basedir_handle, component_filename, NULL, 
                        &sattr, &res_handle, &created);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* check that created == 1 */
    assert(created);

    ret = zoidfs_create(&basedir_handle, component_filename, NULL,
                        &sattr, &res_handle, &created);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* check that created == 0 */
    assert(created == 0);

    ret = zoidfs_create(NULL, NULL, fullpath_filename,
                        &sattr, &res_handle, &created);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    assert(created);

    ret = zoidfs_create(NULL, NULL, fullpath_filename,
                         &sattr, &res_handle, &created);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    assert(created == 0);

    /* try lookup with handle and component */
    ret = zoidfs_lookup(&basedir_handle, component_filename, NULL,
                        &res_handle);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    sattr.mask = ZOIDFS_ATTR_MODE;
    sattr.mode = 0666;

    /* set some attributes */
    ret = zoidfs_setattr(&res_handle, &sattr, &resattr);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* verify attribs were set correctly */
    assert(resattr.mode = 0666);

    /* resize */
    ret = zoidfs_resize(&res_handle, 1000);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* now check the size */
    ret = zoidfs_getattr(&res_handle, &resattr);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* correct size? */
    assert(resattr.mask & ZOIDFS_ATTR_SIZE && resattr.size == 1000);

    ret = zoidfs_rename(&basedir_handle, component_filename, NULL, NULL, NULL,
                        new_fullpath_filename, NULL, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_rename(NULL, NULL, new_fullpath_filename, &basedir_handle,
                        component_filename, NULL, NULL, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* create symlinks */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    ret = zoidfs_symlink(NULL, NULL, symlink, NULL, NULL, symlink_target,
                         &sattr, NULL, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* readlink */
    ret = zoidfs_lookup(NULL, NULL, symlink, &res_handle);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_readlink(&res_handle, buffer, BUFSIZE);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_remove(NULL, NULL, symlink, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }
 
    ret = zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_rename(NULL, NULL, fullpath_filename, NULL, NULL,
                        new_fullpath_filename, NULL, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_remove(NULL, NULL, new_fullpath_filename, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }
 
    dsattr.mask = ZOIDFS_ATTR_ALL;
    dsattr.mode = 0755;
    dsattr.uid = getuid();
    dsattr.gid = getgid();

    gettimeofday(&now, NULL);
    dsattr.atime.seconds = now.tv_sec;
    dsattr.atime.nseconds = now.tv_usec;
    dsattr.mtime.seconds = now.tv_sec;
    dsattr.mtime.nseconds = now.tv_usec;

    /* Create directories */
    ret=zoidfs_mkdir(&basedir_handle, component_filename, NULL, &dsattr, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_mkdir(NULL, NULL, fullpath_filename, &dsattr, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    /* List dirents */
    entries = malloc(entry_count * sizeof(zoidfs_dirent_t));
    if (!entries) {
        perror("malloc() failed");
        goto exit;
    }
    memset(entries, 0, entry_count * sizeof(zoidfs_dirent_t));
    ret = zoidfs_readdir(&basedir_handle, cookie, &entry_count, entries, flags,
                         NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }
    free(entries);

    /* Remove directories */
    ret = zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }

    ret = zoidfs_remove(NULL, NULL, fullpath_filename, NULL);
    if(ret != ZFS_OK)
    {
        goto exit;
    }
 
exit:

    zoidfs_finalize();

    if(ret != ZFS_OK)
    {
	fprintf(stderr, "test failed: error: %d\n", ret);
    }
    else
    {
	fprintf(stdout, "test succeeded.\n");
    }

    return ret;
}


/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End: 
 *
 * vim: ts=8 sts=4 sw=4 expandtab
 */
