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

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"
#include "zoidfs/client/bmi_comm.h"
#include "zoidfs/client/zoidfs_xdr.h"

#define NAMESIZE 255
#define BUFSIZE (8 * 1024 * 1024)

int main(int argc, char **argv) {
    int ret, created;
    struct timeval now;
    size_t mem_sizes[1];
    zoidfs_sattr_t sattr;
    char filename[NAMESIZE];
    size_t mem_count, file_count;
    zoidfs_handle_t handle, basedir_handle;
    uint64_t file_sizes[1], file_starts[1];
    void *mem_starts_write[1], *mem_starts_read[1];

    if(argc < 2) {
        fprintf(stderr, "Usage: zoidfs-io <pvfs2 mount point>\n");
        exit(1);
    }

    mem_count = file_count = 1;

    memset(filename, 0, NAMESIZE);
    snprintf(filename, NAMESIZE, "zoidfs-io.dat");

    ret = zoidfs_init();
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Allocate memory for read and write buffers */
    mem_starts_read[0] = malloc(BUFSIZE);
    if (!mem_starts_read[0]) {
        fprintf(stderr, "zoidfs-io: malloc() failed.\n");
        exit(1);
    }
    mem_starts_write[0] = malloc(BUFSIZE);
    if (!mem_starts_write[0]) {
        fprintf(stderr, "zoidfs-io: malloc() failed.\n");
        exit(1);
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
    memset(mem_starts_write[0], 1, BUFSIZE);
    file_sizes[0] = mem_sizes[0] = BUFSIZE;
    file_starts[0] = 0;

    ret = zoidfs_write(&handle, mem_count, (const void **)mem_starts_write,
                       mem_sizes, file_count, file_starts, file_sizes);
    if(ret != ZFS_OK) {
        goto exit;
    }

    ret = zoidfs_commit(&handle);
    if(ret != ZFS_OK) {
        goto exit;
    }

    /* Read the data back */
    file_sizes[0] = mem_sizes[0] = BUFSIZE;
    memset(mem_starts_read[0], 0, BUFSIZE);
    ret = zoidfs_read(&handle, mem_count, (void **)mem_starts_read, mem_sizes,
                      file_count, file_starts, file_sizes);
    if(ret != ZFS_OK) {
        goto exit;
    }

    assert(memcmp(mem_starts_write[0], mem_starts_read[0], BUFSIZE) == 0);

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

    free(mem_starts_read[0]);
    free(mem_starts_write[0]);

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
