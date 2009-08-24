#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

/* interface symbol magic */
#define _PREFIX_IOFSL_INTERFACE(__iofslprefix, __iofslfunc) \
    __iofslprefix ## _ ## __iofslfunc
#define PREFIX_IOFSL_INTERFACE(__iofslprefix, __iofslfunc) \
    _PREFIX_IOFSL_INTERFACE(__iofslprefix, __iofslfunc)

#ifdef IOFSL_ALT_SYMBOL
#define IOFSL_INTERFACE(__iofslfunc) PREFIX_IOFSL_INTERFACE(IOFSL_ALT_SYMBOL, __iofslfunc)
#else
#define IOFSL_INTERFACE(__func) __func 
#endif

int main()
{
    //_iofwd_sysio_startup();

    /* open tests */
    int fd1 = IOFSL_INTERFACE(open)("/sysio/sysio.test.txt", O_RDWR | O_CREAT, 0644);
    int fd2 = IOFSL_INTERFACE(open)("/sysio/sysio.test.txt.1", O_RDWR | O_CREAT, 0644);
    int fd3 = IOFSL_INTERFACE(open)("/sysio/sysio.test.txt.2", O_RDWR | O_CREAT, 0644);
    int fd4 = IOFSL_INTERFACE(open)("/sysio/sysio.test.txt.3", O_RDWR | O_CREAT, 0644);
    int fd5 = IOFSL_INTERFACE(open)("/sysio/sysio.test.txt.4", O_RDWR | O_CREAT, 0644);
    int fd6 = IOFSL_INTERFACE(open)("/sysio/read.sysio.test", O_RDWR | O_CREAT, 0644);
    int fd7 = IOFSL_INTERFACE(open)("/sysio/write.sysio.test", O_RDWR | O_CREAT, 0644);

    /* rename tests */
    //_iofwd_rename("/sysio/sysio.test.txt", "/sysio/sysio.test.txt");
    //_iofwd_rename("/sysio/sysio.test.txt.rename", "/sysio/sysio.test.txt");

    /* read test */
    char readbuffer[1024];
    memset(readbuffer, 0, 1024);
    IOFSL_INTERFACE(read)(fd6, readbuffer, 1024);
    fprintf(stderr, "File Contents: %s\n", readbuffer);

    /* write test */
    char writebuffer[1024];
    memset(writebuffer, 0, 1024);
    sprintf(writebuffer, "blahblahblahblahend this test now!\n");
    IOFSL_INTERFACE(write)(fd7, writebuffer, strlen(writebuffer) + 1);
    fprintf(stderr, "File Contents: %s\n", writebuffer);

    /* unlink test */
    IOFSL_INTERFACE(unlink)("/sysio/sysio.test.txt");
    IOFSL_INTERFACE(unlink)("/sysio/sysio.test.txt.1");

    /* mkdir test */
    IOFSL_INTERFACE(mkdir)("/sysio/subdir", 0755);

    /* rmdir */
    IOFSL_INTERFACE(rmdir)("/sysio/subdir");

    /* stat tests */
    struct stat sbuffer;
    IOFSL_INTERFACE(stat)("/sysio/sysio.test.txt.2", &sbuffer);
    fprintf(stderr, "uid = %i, gid = %i\n", sbuffer.st_uid, sbuffer.st_gid);
    fprintf(stderr, "mode = %o\n", sbuffer.st_mode & 0777);
    IOFSL_INTERFACE(chmod)("/sysio/sysio.test.txt.2", 0755);
    IOFSL_INTERFACE(stat)("/sysio/sysio.test.txt.2", &sbuffer);
    fprintf(stderr, "mode = %o\n", sbuffer.st_mode & 0777);
    IOFSL_INTERFACE(unlink)("/sysio/sysio.test.txt.2");

    /* link tests */
    /*_iofwd_link("/sysio/sysio.test.txt.3", "/sysio/sysio.link.test");
    _iofwd_unlink("/sysio/sysio.link.test");*/
    IOFSL_INTERFACE(unlink)("/sysio/sysio.test.txt.3");

    /* symlink tests */
    IOFSL_INTERFACE(symlink)("/sysio/sysio.test.txt.4", "/sysio/sysio.test.txt.sl");
    IOFSL_INTERFACE(unlink)("/sysio/sysio.test.txt.4");
    IOFSL_INTERFACE(unlink)("/sysio/sysio.test.txt.sl");

    /* readlink tests */
    char rbuffer[4096];
    int ret = 0;
    ret = IOFSL_INTERFACE(readlink)("/etc/hosts", rbuffer, 4096);
    fprintf(stderr, "%s\n", rbuffer);

    /* close tests */
    IOFSL_INTERFACE(close)(fd1);
    IOFSL_INTERFACE(close)(fd2);
    IOFSL_INTERFACE(close)(fd3);
    IOFSL_INTERFACE(close)(fd4);
    IOFSL_INTERFACE(close)(fd5);

    return 0;
}
