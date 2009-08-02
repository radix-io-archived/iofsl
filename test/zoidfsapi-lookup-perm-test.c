/*
 * zoidfsapi-lookup-perms.c
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
    int ret;
    zoidfs_handle_t basedir_handle;
    zoidfs_handle_t fhandle;
    char lookup_component_orig[NAMESIZE], lookup_fullpath_orig[NAMESIZE];

    if(argc < 1) {
        fprintf(stderr, "Usage: zoidfsapi_lookup_perms\n");
        exit(1);
    }

    ret = zoidfs_init();
    if(ret != ZFS_OK) {
        return ret;
    }

    /* lookup the basedir */
    ret = zoidfs_lookup(NULL, NULL, "/etc", &basedir_handle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /*
     * Try to get a handle for a file that we can't access... like
     * /etc/shadow 
     */
    /* path variables */
    snprintf(lookup_fullpath_orig, NAMESIZE, "%s/shadow", "/etc");
    snprintf(lookup_component_orig, NAMESIZE, "shadow");

    ret = zoidfs_lookup(&basedir_handle, lookup_component_orig, NULL, &fhandle);
    fprintf(stderr, "/etc/shadow test\n");
    if(ret != ZFS_OK) {
        fprintf(stderr, "component handle: could not access file with restrictive permissions\n");
    }
    else
    {
        fprintf(stderr, "component handle: could access file with restrictive permissions\n");
    }

    ret = zoidfs_lookup(NULL, NULL, lookup_fullpath_orig, &fhandle);
    if(ret != ZFS_OK) {
        fprintf(stderr, "fullpath handle: could not access file with restrictive permissions\n");
    }
    else
    {
        fprintf(stderr, "fullpath handle: could access file with restrictive permissions\n");
    }

    /*
     * Try to get a handle for a file that we can't access... like
     * /etc/group 
     */
    /* path variables */
    snprintf(lookup_fullpath_orig, NAMESIZE, "%s/group", "/etc");
    snprintf(lookup_component_orig, NAMESIZE, "group");

    ret = zoidfs_lookup(&basedir_handle, lookup_component_orig, NULL, &fhandle);
    fprintf(stderr, "/etc/group test\n");
    if(ret != ZFS_OK) {
        fprintf(stderr, "component handle: could not access file with nonrestrictive permissions\n");
    }
    else
    {
        fprintf(stderr, "component handle: could access file with nonrestrictive permissions\n");
    }

    ret = zoidfs_lookup(NULL, NULL, lookup_fullpath_orig, &fhandle);
    if(ret != ZFS_OK) {
        fprintf(stderr, "fullpath handle: could not access file with nonrestrictive permissions\n");
    }
    else
    {
        fprintf(stderr, "fullpath handle: could access file with nonrestrictive permissions\n");
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
