#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>

#include <zoidfs/zoidfs.h>
#include <zoidfs/hints/zoidfs-hints.h>

int main()
{
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_cache_hint_t parent_hint;

    /* set the attrs */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0755;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    zoidfs_op_hint_t hint;

    /* create the hint */
    zoidfs_hint_create(&hint);

    /* set some hint data */
    zoidfs_hint_set(hint, strdup("key"), strdup("data"), 5);
    zoidfs_hint_set(hint, strdup("key1"), strdup("data1"), 6);
    zoidfs_hint_set(hint, strdup("key2"), strdup("data2"), 6);
    zoidfs_hint_set(hint, strdup("key3"), strdup("data3"), 6);
    zoidfs_hint_set(hint, strdup("key4"), strdup("data4"), 6);
    zoidfs_hint_set(hint, strdup("key5"), strdup("data5"), 6);
    zoidfs_hint_set(hint, strdup("key5"), strdup("data5"), 6);
    zoidfs_hint_set(hint, strdup("key6"), strdup("data6"), 6);
    zoidfs_hint_set(hint, strdup("key7"), strdup("data7"), 6);

    zoidfs_init();
    
    zoidfs_mkdir(NULL, NULL, "/test-dir", &sattr, &parent_hint, ZOIDFS_NO_OP_HINT);
    zoidfs_mkdir(NULL, NULL, "/test-dir-hints", &sattr, &parent_hint, &hint);
    zoidfs_mkdir(NULL, NULL, "/test-dir2", &sattr, &parent_hint, ZOIDFS_NO_OP_HINT);
    zoidfs_mkdir(NULL, NULL, "/test-dir-hints2", &sattr, &parent_hint, &hint);
   
    zoidfs_remove(NULL, NULL, "/test-dir", NULL, &hint); 
    zoidfs_remove(NULL, NULL, "/test-dir-hints", NULL, &hint); 
    zoidfs_remove(NULL, NULL, "/test-dir2", NULL, &hint); 
    zoidfs_remove(NULL, NULL, "/test-dir-hints2", NULL, &hint);

    zoidfs_hint_free(&hint);

    zoidfs_finalize();

    zoidfs_hint_free(&hint);

    return 0;
}
