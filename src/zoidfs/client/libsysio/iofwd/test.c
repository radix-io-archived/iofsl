#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main()
{
    //_iofwd_sysio_startup();

    /* open tests */
    int fd1 = _iofwd_open("/sysio/sysio.test.txt", O_RDWR | O_CREAT, 0644);
    int fd2 = _iofwd_open("/sysio/sysio.test.txt.1", O_RDWR | O_CREAT, 0644);
    //int fd3 = _iofwd_open("/sysio/sysio.test.txt.2", O_RDWR, 0644);
    //int fd4 = _iofwd_open("/sysio/subdir/sysio.test.txt.4", O_RDWR | O_CREAT, 0644);
    //int fd5 = _iofwd_open("/sysio/subdir/sysio.test.txt.5", O_RDWR, 0644);

    /* rename tests */
    //_iofwd_rename("/sysio/sysio.test.txt", "/sysio/sysio.test.txt");
    //_iofwd_rename("/sysio/sysio.test.txt.rename", "/sysio/sysio.test.txt");

    /* unlink test */
    _iofwd_unlink("/sysio/sysio.test.txt");
    _iofwd_unlink("/sysio/sysio.test.txt.1");

    /* mkdir test */
    //_iofwd_mkdir("/sysio/subdir", 0755);

    /* stat test */
    //struct stat sbuffer;
    //_iofwd_stat("/sysio/sysio.test.txt", &sbuffer);

    /* close tests */
    _iofwd_close(fd1);
    _iofwd_close(fd2);
    //_iofwd_close(fd3);
    //_iofwd_close(fd4);
    //_iofwd_close(fd5);

    return 0;
}
