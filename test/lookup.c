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

int main(int argc, char * args[])
{
    zoidfs_handle_t handle;
    zoidfs_sattr_t sattr;
    int created = 0;

    zoidfs_init();

    zoidfs_lookup(NULL, NULL, "/tmp/gftptest.txt", &handle, ZOIDFS_NO_OP_HINT);
    zoidfs_lookup(NULL, NULL, "/tmp/gftptest.txt.not", &handle, ZOIDFS_NO_OP_HINT);

    zoidfs_create(NULL, NULL, "/tmp/gftp.create.test.txt", &sattr, &handle, &created, ZOIDFS_NO_OP_HINT);
 
    zoidfs_remove(NULL, NULL, "/tmp/gftptest.txt.rm", NULL, ZOIDFS_NO_OP_HINT);

    zoidfs_finalize();

    return 0;
}
