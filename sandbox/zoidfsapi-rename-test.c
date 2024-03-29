/*
 * zoidfsapi-rename-test.c
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
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_handle_t basedir_handle;
    zoidfs_handle_t fhandle, rhandle;
    char rename_component_orig[NAMESIZE], rename_fullpath_orig[NAMESIZE];
    char rename_component_target[NAMESIZE], rename_fullpath_target[NAMESIZE];

    if(argc < 2) {
        fprintf(stderr, "Usage: zoidfsapi_rename_test <mount point>\n");
        exit(1);
    }

    /* path variables */

    snprintf(rename_fullpath_target, NAMESIZE, "%s/rename", argv[1]);
    snprintf(rename_component_target, NAMESIZE, "rename-comp");
    snprintf(rename_fullpath_orig, NAMESIZE, "%s/test-zoidfs-fullpath", argv[1]);
    snprintf(rename_component_orig, NAMESIZE, "test-zoidfs-component");

    ret = zoidfs_init();
    if(ret != ZFS_OK) {
        return ret;
    }

    /* lookup the basedir */
    ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /*
     * Rename using base handle and component name
     */

    /* If comp file exists, delete it */
    ret = zoidfs_lookup(&basedir_handle, rename_component_orig, NULL, &fhandle);
    if(ret == ZFS_OK) {
        ret = zoidfs_remove(&basedir_handle, rename_component_orig, NULL, NULL);
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
    ret = zoidfs_create(&basedir_handle, rename_component_orig, NULL, &sattr, &fhandle, &created);
    if(ret != ZFS_OK) {
        goto exit;
    }
    /* Check that created == 1 */
    assert(created);
   
    /*
     * Get handle for the original file
     */ 
    ret = zoidfs_lookup(&basedir_handle, rename_component_orig, NULL, &fhandle);
    if(ret != ZFS_OK) {
            goto exit;
    }

    /* create renames using the component path*/
    ret = zoidfs_rename(&basedir_handle, rename_component_orig, NULL, &basedir_handle, rename_component_target, NULL,
                         NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_lookup(&basedir_handle, rename_component_target, NULL, &rhandle);
    if(ret != ZFS_OK) {
            goto exit;
    }

    if(memcmp(&rhandle, &fhandle, sizeof(zoidfs_handle_t)) == 0)
    {
        fprintf(stderr, "component test: rename handle and original file handle are the same\n");
    }
    else
    {
        fprintf(stderr, "component test: rename handle and original file handle differ\n");
    }

    /*
     * Rename using fullpath name
     */

    /* Try create with fullpath */
    ret = zoidfs_create(NULL, NULL, rename_fullpath_orig, &sattr, &fhandle, &created);
    if(ret != ZFS_OK) {
        goto exit;
    }
    /* Check that created == 1 */
    assert(created);

    /* Try lookup with full_path */
    ret = zoidfs_lookup(NULL, NULL, rename_fullpath_orig, &fhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* create renames  using the full path*/
    ret = zoidfs_rename(NULL, NULL, rename_fullpath_orig, NULL, NULL, rename_fullpath_target,
                         NULL, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Try lookup with component */
    ret = zoidfs_lookup(NULL, NULL, rename_fullpath_target, &rhandle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    if(memcmp(&rhandle, &fhandle, sizeof(zoidfs_handle_t)) == 0)
    {
        fprintf(stderr, "fullpath handle: rename handle and original file handle are the same\n");
    }
    else
    {
        fprintf(stderr, "fullpath handle: rename handle and original file handle differ\n");
    }

    /* Remove the files */
    ret = zoidfs_remove(NULL, NULL, rename_fullpath_target, NULL);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_remove(&basedir_handle, rename_component_target, NULL, NULL);
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
