/*
 * zoidfs-io.c
 * Test the ZOIDFS I/O interface.
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

#define NAMESIZE 255

int main(int argc, char **argv) {
    char **mem_starts;
    struct timeval now;
    int ret, created, i;
    zoidfs_sattr_t sattr;
    char filename[NAMESIZE];
    size_t mem_sizes[1024];
    int mem_count, file_count = 1;
    zoidfs_handle_t handle, basedir_handle;
    char str1[] = "hello ", str2[] = "world";
    uint64_t file_sizes[1024], file_starts[1024];

    if(argc < 2) {
        fprintf(stderr, "Usage: zoidfs-io <pvfs2 mount point>\n");
        exit(1);
    }

    mem_count = file_count;

    memset(filename, 0, NAMESIZE);
    snprintf(filename, NAMESIZE, "zoidfs-io.dat");

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

    ret = zoidfs_lookup(&basedir_handle, filename, NULL, &handle);
    if(ret == ZFS_OK) {
        /* If it exists, we delete it */
        ret = zoidfs_remove(&basedir_handle, filename, NULL, NULL);
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

    /* Create a file */
    ret = zoidfs_create(&basedir_handle, filename, NULL, &sattr, &handle,
                        &created);
    if(ret != ZFS_OK) {
        goto exit;
    }
    assert(created);

    /* Write to the file */
    mem_starts = malloc(mem_count * sizeof(char *));
    if (!mem_starts) {
        perror("zoidfs-io: malloc() failed");
        exit(1);
    }

    for (i = 0; i < mem_count; i++) {
        file_starts[i] = 0;
        mem_starts[i] = malloc(1024 * sizeof(char));
        if (!mem_starts[i]) {
            perror("zoidfs-io: malloc() failed");
            exit(1);
        }
        memset(mem_starts[i], 0, 1024);
        strcpy(mem_starts[i], str1);
        mem_sizes[i] = strlen(str1);
        file_sizes[i] = mem_sizes[i];
    }

    ret = zoidfs_write(&handle, mem_count, (const void **)mem_starts,
                       mem_sizes, file_count, file_starts, file_sizes);
    if(ret != ZFS_OK) {
        goto exit;
    }

    for (i = 0; i < mem_count; i++) {
        file_starts[i] = strlen(str1);
        memset(mem_starts[i], 0, 1024);
        strcpy(mem_starts[i], str2);
        mem_sizes[i] = strlen(str2);
        file_sizes[i] = mem_sizes[i];
    }

    ret = zoidfs_write(&handle, mem_count, (const void **)mem_starts,
                       mem_sizes, file_count, file_starts, file_sizes);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Remove the file */
    ret = zoidfs_remove(&basedir_handle, filename, NULL, NULL);
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

/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
