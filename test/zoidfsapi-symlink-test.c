/*
 * zoidfsapi-symlink-test.c
 * Test the ZOIDFS symlink test.
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

int compare_attr(zoidfs_attr_t * file, zoidfs_attr_t * link)
{

#define CMP_ZFS_ATTR(_i) \
    if(file->_i != link->_i) \
        return 0

    CMP_ZFS_ATTR(nlink);
    CMP_ZFS_ATTR(size);
    CMP_ZFS_ATTR(mode);
    CMP_ZFS_ATTR(blocksize);
    CMP_ZFS_ATTR(uid);
    CMP_ZFS_ATTR(gid);
    CMP_ZFS_ATTR(atime.seconds);
    CMP_ZFS_ATTR(ctime.seconds);
    CMP_ZFS_ATTR(mtime.seconds);
    CMP_ZFS_ATTR(atime.nseconds);
    CMP_ZFS_ATTR(ctime.nseconds);
    CMP_ZFS_ATTR(mtime.nseconds);

    return 1;
}

int main(int argc, char **argv) {
    int ret, created;
    uint32_t flags = 0;
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_attr_t resattr_file;
    zoidfs_attr_t resattr_link;
    zoidfs_handle_t basedir_handle;
    zoidfs_handle_t fhandle, shandle;
    char symlink[NAMESIZE], symlink_local[NAMESIZE], symlink_target[NAMESIZE];
    char symlink_component_target[NAMESIZE], symlink_fullpath_target[NAMESIZE];
    char fullpath_filename[NAMESIZE], component_filename[NAMESIZE];

    if(argc < 2) {
        fprintf(stderr, "Usage: zoidfsapi_symlink_test <mount point>\n");
        exit(1);
    }

    /* path variables */

    snprintf(symlink, NAMESIZE, "%s/symlink", argv[1]);
    snprintf(symlink_local, NAMESIZE, "symlink-comp");
    snprintf(symlink_fullpath_target, NAMESIZE, "%s/test-zoidfs-fullpath", argv[1]);
    snprintf(symlink_component_target, NAMESIZE, "test-zoidfs-component");
    snprintf(fullpath_filename, NAMESIZE, "%s/test-zoidfs-fullpath", argv[1]);
    snprintf(component_filename, NAMESIZE, "test-zoidfs-component");

    ret = zoidfs_init();
    if(ret != ZFS_OK) {
        return ret;
    }

    /* lookup the basedir */
    ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* If comp file exists, delete it */
    ret = zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    if(ret == ZFS_OK) {
        ret = zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);
        if(ret != ZFS_OK) {
            goto exit;
        }
    }

    /* If fullpath file exists, delete it */
    ret = zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    if(ret == ZFS_OK) {
        ret = zoidfs_remove(NULL, NULL, fullpath_filename, NULL);
        if(ret != ZFS_OK) {
            goto exit;
        }
    }

    /* file attrs */
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
    ret = zoidfs_create(&basedir_handle, component_filename, NULL, &sattr, &fhandle, &created);
    if(ret != ZFS_OK) {
        goto exit;
    }
    /* Check that created == 1 */
    assert(created);

    /* create symlinks  using the component path*/
    ret = zoidfs_symlink(&basedir_handle, symlink_local, NULL, &basedir_handle, symlink_component_target, NULL,
                         &sattr, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Try create with fullpath */
    ret = zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &fhandle, &created);
    if(ret != ZFS_OK) {
        goto exit;
    }
    /* Check that created == 1 */
    assert(created);

    /* Try lookup with full_path */
    ret = zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* create symlinks  using the full path*/
    ret = zoidfs_symlink(NULL, NULL, symlink, NULL, NULL, symlink_fullpath_target,
                         &sattr, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_lookup(NULL, NULL, symlink, &shandle);
    if(ret != ZFS_OK) {
            goto exit;
    }

    if(memcmp(&shandle, &fhandle, sizeof(zoidfs_handle_t)) == 0)
    {
        fprintf(stderr, "fullpath test: symlink handle and original file handle are the same\n");
    }
    else
    {
        fprintf(stderr, "fullpath test: symlink handle and original file handle differ\n");
    }

    /* Now check the attrs of the symlink and the file handle */
    resattr_file.mask = ZOIDFS_ATTR_ALL;
    resattr_link.mask = ZOIDFS_ATTR_ALL;
    ret = zoidfs_getattr(&shandle, &resattr_link);
    if(ret != ZFS_OK) {
        goto exit;
    }
    ret = zoidfs_getattr(&fhandle, &resattr_file);
    if(ret != ZFS_OK) {
        goto exit;
    }

    if(compare_attr(&resattr_file, &resattr_link))
    {
        fprintf(stderr, "fullpath test: file attributes are the same for the symlink and the original file\n");
    }
    else
    {
        fprintf(stderr, "fullpath test: file attributes differ for the symlink and the original file\n");
    }

    /* Try lookup with component */
    ret = zoidfs_lookup(&basedir_handle, component_filename, NULL, &fhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_lookup(&basedir_handle, symlink_local, NULL, &shandle);
    if(ret != ZFS_OK) {
            goto exit;
    }

    if(memcmp(&shandle, &fhandle, sizeof(zoidfs_handle_t)) == 0)
    {
        fprintf(stderr, "component handle: symlink handle and original file handle are the same\n");
    }
    else
    {
        fprintf(stderr, "component handle: symlink handle and original file handle differ\n");
    }

    /* Now check the attrs of the symlink and the file handle */
    resattr_file.mask = ZOIDFS_ATTR_ALL;
    resattr_link.mask = ZOIDFS_ATTR_ALL;
    ret = zoidfs_getattr(&shandle, &resattr_link);
    if(ret != ZFS_OK) {
        goto exit;
    }
    ret = zoidfs_getattr(&fhandle, &resattr_file);
    if(ret != ZFS_OK) {
        goto exit;
    }

    if(compare_attr(&resattr_file, &resattr_link))
    {
        fprintf(stderr, "component test: file attributes are the same for the symlink and the original file\n");
    }
    else
    {
        fprintf(stderr, "component test: file attributes differ for the symlink and the original file\n");
    }

    /* Remove the files and symlinks */
    ret = zoidfs_remove(&basedir_handle, component_filename, NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_remove(NULL, NULL, fullpath_filename, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_remove(NULL, NULL, symlink, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_remove(&basedir_handle, symlink_local, NULL, NULL);
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
