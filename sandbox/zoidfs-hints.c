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

    zoidfs_op_hint_t * hint  = zoidfs_hint_init(3);

    zoidfs_hint_add(&hint, strdup("key"), strdup("data"), 5, ZOIDFS_HINTS_ZC);
    zoidfs_hint_add(&hint, strdup("key1"), strdup("data1"), 6, ZOIDFS_HINTS_ZC);
    zoidfs_hint_add(&hint, strdup("key2"), strdup("data2"), 6, ZOIDFS_HINTS_ZC);
    zoidfs_hint_print(&hint);
    zoidfs_hint_add(&hint, strdup("key3"), strdup("data3"), 6, ZOIDFS_HINTS_ZC);
    zoidfs_hint_add(&hint, strdup("key4"), strdup("data4"), 6, ZOIDFS_HINTS_ZC);
    zoidfs_hint_add(&hint, strdup("key5"), strdup("data5"), 6, ZOIDFS_HINTS_ZC);
    zoidfs_hint_add(&hint, strdup("key6"), strdup("data6"), 6, ZOIDFS_HINTS_ZC);
    zoidfs_hint_add(&hint, strdup("key7"), strdup("data7"), 6, ZOIDFS_HINTS_ZC);

    zoidfs_hint_print(&hint);
    
    zoidfs_hint_remove(&hint, "key4");
    zoidfs_hint_print(&hint);

    zoidfs_hint_remove(&hint, "key4");
    zoidfs_hint_print(&hint);

    zoidfs_hint_remove(&hint, "key7");
    zoidfs_hint_print(&hint);

    zoidfs_hint_remove(&hint, "key");
    zoidfs_hint_print(&hint);

    zoidfs_hint_remove(&hint, "key3");
    zoidfs_hint_print(&hint);

    //zoidfs_hint_add(&hint, strdup("key"), strdup("data"), 5, ZOIDFS_HINTS_ZC);
    zoidfs_hint_print(&hint);

    zoidfs_hint_add(&hint, strdup("key"), strdup("data4"), 6, ZOIDFS_HINTS_ZC);
    zoidfs_hint_print(&hint);

    zoidfs_op_hint_t * _hint = zoidfs_hint_pop(&hint);
    fprintf(stderr, "hint pop: key = %s, value = %s\n", _hint->key, _hint->value);
    zoidfs_hint_print(&hint);
    zoidfs_hint_destroy(&_hint);

    fprintf(stderr, "hint list size: size = %i\n", zoidfs_hint_num_elements(&hint));

    zoidfs_init();

    zoidfs_mkdir(NULL, NULL, "/test-dir", &sattr, &parent_hint, ZOIDFS_NO_OP_HINT);
    zoidfs_mkdir(NULL, NULL, "/test-dir-hints", &sattr, &parent_hint, hint);
    zoidfs_mkdir(NULL, NULL, "/test-dir2", &sattr, &parent_hint, ZOIDFS_NO_OP_HINT);
    zoidfs_mkdir(NULL, NULL, "/test-dir-hints2", &sattr, &parent_hint, hint);

    zoidfs_finalize();

    zoidfs_hint_destroy(&hint);

    return 0;
}
