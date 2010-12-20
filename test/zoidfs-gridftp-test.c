#include <time.h>
#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

#include "zoidfs/zoidfs.h"

int main()
{
    zoidfs_handle_t handle;
    zoidfs_handle_t lhandle;
    zoidfs_handle_t phandle;
    zoidfs_sattr_t sattr;
    zoidfs_attr_t attr;
    int created = 0;
    char ** mem;
    size_t * memsizes;
    char ** memcp;
    size_t * memsizescp;
    uint64_t * file;
    uint64_t * filesizes;
    size_t off = 0;
    size_t i = 0;
    const size_t asize = 50;

    file = malloc(sizeof(uint64_t) * asize);
    filesizes = malloc(sizeof(uint64_t) * asize);
    mem = malloc(sizeof(char *) * asize);
    memsizes = malloc(sizeof(size_t) * asize);

    memcp = malloc(sizeof(char *) * asize);
    memsizescp = malloc(sizeof(size_t) * asize);

    for(i = 0 ; i < asize ; i++)
    {
        char tmps[1024];

        memset(tmps, '\0', 1024);
        sprintf(tmps, "this is a test %lu", i);

        mem[i] = strdup(tmps);
        memsizes[i] = strlen(mem[i]) + 1;
        memcp[i] = (char *)malloc(sizeof(char) * memsizes[i]);
        memset(memcp[i], '\0', memsizes[i]);
        memsizescp[i] = memsizes[i];

        file[i] = off;
        filesizes[i] = strlen(mem[i]) + 1;

        off += filesizes[i];
    }

    zoidfs_init();

    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp", &phandle, ZOIDFS_NO_OP_HINT);
    zoidfs_lookup(&phandle, "/gftptest.txt", NULL, &phandle, ZOIDFS_NO_OP_HINT);
    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftptest.txt", &handle, ZOIDFS_NO_OP_HINT);
    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftptest.txt", &handle, ZOIDFS_NO_OP_HINT);
    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftptest.txt", &handle, ZOIDFS_NO_OP_HINT);
    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftptest.txt.not", &handle, ZOIDFS_NO_OP_HINT);

    zoidfs_create(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftp.create.test.txt", &sattr, &handle, &created, ZOIDFS_NO_OP_HINT);
    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftp.create.test.txt", &lhandle, ZOIDFS_NO_OP_HINT);
    zoidfs_getattr(&lhandle, &attr, ZOIDFS_NO_OP_HINT);
    zoidfs_getattr(&lhandle, &attr, ZOIDFS_NO_OP_HINT);

    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftp.getattr.test.txt", &lhandle, ZOIDFS_NO_OP_HINT);
    zoidfs_getattr(&lhandle, &attr, ZOIDFS_NO_OP_HINT);
    zoidfs_getattr(&lhandle, &attr, ZOIDFS_NO_OP_HINT);
 
    zoidfs_remove(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftptest.txt.rm", NULL, ZOIDFS_NO_OP_HINT);

    zoidfs_mkdir(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftptest.dir", &sattr, NULL, ZOIDFS_NO_OP_HINT);

    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftpresize.txt.1", &handle, ZOIDFS_NO_OP_HINT);
    zoidfs_resize(&handle, 1024, ZOIDFS_NO_OP_HINT);
    zoidfs_resize(&handle, 1023, ZOIDFS_NO_OP_HINT);
    zoidfs_resize(&handle, 1022, ZOIDFS_NO_OP_HINT);
    zoidfs_resize(&handle, 1021, ZOIDFS_NO_OP_HINT);

    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftpresize.txt.2", &handle, ZOIDFS_NO_OP_HINT);
    zoidfs_resize(&handle, 16000, ZOIDFS_NO_OP_HINT);

    zoidfs_lookup(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftpwrite.txt", &handle, ZOIDFS_NO_OP_HINT);
    zoidfs_write(&handle, asize, (const void**) mem, memsizes, asize, file,
          filesizes, ZOIDFS_NO_OP_HINT);

    zoidfs_read(&handle, asize, (void**) memcp, memsizescp, asize, file,
          filesizes, ZOIDFS_NO_OP_HINT);

    for(i = 0 ; i < asize ; i++)
    {
        fprintf(stderr, "read data = %s\n", memcp[i]);
    }

    //zoidfs_rename(NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftprename.txt", NULL, NULL, "/ftp/127.0.0.1/2811/tmp/gftprename.txt.mod", NULL, NULL, ZOIDFS_NO_OP_HINT);

    zoidfs_finalize();

    for(i = 0 ; i < asize ; i++)
    {
        free(mem[i]);
        free(memcp[i]);
    }

    free(mem);
    free(memsizes);
    free(memsizescp);
    free(file);
    free(filesizes);

    return 0;
}
