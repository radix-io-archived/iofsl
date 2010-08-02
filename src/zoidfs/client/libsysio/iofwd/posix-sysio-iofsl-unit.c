#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

/* interface symbol magic */
#define _PREFIX_IOFSL_INTERFACE(__iofslprefix, __iofslfunc) \
    __iofslprefix ## _ ## __iofslfunc
#define PREFIX_IOFSL_INTERFACE(__iofslprefix, __iofslfunc) \
    _PREFIX_IOFSL_INTERFACE(__iofslprefix, __iofslfunc)

#ifdef IOFSL_ALT_SYMBOL
#define IOFSL_INTERFACE(__iofslfunc) PREFIX_IOFSL_INTERFACE(IOFSL_ALT_SYMBOL, __iofslfunc)
#else
#define IOFSL_INTERFACE(__iofslfunc) PREFIX_IOFSL_INTERFACE(_zfs_sysio, __iofslfunc)
//#define IOFSL_INTERFACE(__func) __func 
#endif

int open_close_unlink_test(char * base_path)
{
    int fd1 = 0, fd2 = 0, fd3 = 0;
    int err = 0;
    char tf1[4096];
    char tf2[4096];
    char tf3[4096];
    char tfe[4096];

    fprintf(stderr, "%s : start open / close test\n", __func__);

    /* build the file names */
    sprintf(tf1, "%s/sysio.test.1.txt", base_path);
    sprintf(tf2, "%s/sysio.test.2.txt", base_path);
    sprintf(tf3, "%s/sysio.test.3.txt", base_path);
    sprintf(tfe, "%s/sysio.test.exist.txt", base_path);

    /* open or create files that do not exist */
    fd1 = IOFSL_INTERFACE(open)(tf1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s, %s:%i\n", tf1, __func__, __LINE__);
    }

    fd2 = IOFSL_INTERFACE(open)(tf2, O_RDWR | O_CREAT, 0644);
    if(fd2 == -1)
    {
        fprintf(stderr, "Could not open %s, %s:%i\n", tf2, __func__, __LINE__);
    }

    fd3 = IOFSL_INTERFACE(creat)(tf3, 0600);
    if(fd3 == -1)
    {
        fprintf(stderr, "Could not create  %s, %s:%i\n", tf3, __func__, __LINE__);
    }

    /* close files just created */
    err = IOFSL_INTERFACE(close)(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s, %s:%i\n", tf1, __func__, __LINE__);
    }

    err = IOFSL_INTERFACE(close)(fd2);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s, %s:%i\n", tf2, __func__, __LINE__);
    }

    err = IOFSL_INTERFACE(close)(fd3);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s, %s:%i\n", tf3, __func__, __LINE__);
    }

    fprintf(stderr, "%s : start existing open / close test\n", __func__);
    /* open files that already exist */
    fd1 = IOFSL_INTERFACE(open)(tf1, O_RDWR);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s, %s:%i\n", tf1, __func__, __LINE__);
    }

    err = IOFSL_INTERFACE(close)(fd1);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s, %s:%i\n", tf1, __func__, __LINE__);
    }


    fd2 = IOFSL_INTERFACE(open)(tfe, O_RDONLY);
    if(fd2 == -1)
    {
        fprintf(stderr, "Could not open %s, %s:%i\n", tfe, __func__, __LINE__);
    }

    if(fd2 == -1)
    {
        fprintf(stderr, "Could not open %s, %s:%i\n", tfe, __func__, __LINE__);
    }

    fprintf(stderr, "%s : start unlink test\n", __func__);
    /* unlink the files */
    err = IOFSL_INTERFACE(unlink)(tf1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s, %s:%i\n", tf1, __func__, __LINE__);
    }
 
    err = IOFSL_INTERFACE(unlink)(tf2);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s, %s:%i\n", tf2, __func__, __LINE__);
    }

    err = IOFSL_INTERFACE(unlink)(tf3);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s, %s:%i\n", tf3, __func__, __LINE__);
    }
    return 0;
}

int rename_test(char * base_path)
{
    int fd1 = 0;
    int err = 0;
    char trn1[4096];
    char trn2[4096];

    /* build the file names */
    sprintf(trn1, "%s/sysio.test.rename.txt", base_path);
    sprintf(trn2, "%s/sysio.test.rename.txt.new", base_path);

    fd1 = IOFSL_INTERFACE(open)(trn1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s\n", trn1);
    }

    err = close(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s\n", trn1);
    }

    err = IOFSL_INTERFACE(rename)(trn1, trn2);
    if(err)
    {
        fprintf(stderr, "Could not rename %s to %s\n", trn1, trn2);
    }

    err = IOFSL_INTERFACE(rename)(trn2, trn1);
    if(err)
    {
        fprintf(stderr, "Could not rename %s to %s\n", trn2, trn1);
    }

    err = IOFSL_INTERFACE(unlink)(trn1);
    if(err)
    {
        fprintf(stderr, "Could not unlink %s\n", trn1);
    }
    
    return 0;
}

int symlink_test(char * base_path)
{
    int err = 0;
    int fd1 = 0;
    char trs1[4096];
    char trs2[4096];
  
    /* build the file names */
    sprintf(trs1, "%s/sysio.test.rename.txt", base_path);
    sprintf(trs2, "%s/sysio.test.rename.txt.sl", base_path);
 
    /* open and create the file */ 
    fd1 = IOFSL_INTERFACE(open)(trs1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s\n", trs1);
    }
   
    /* close the fd */ 
    err = close(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s\n", trs1);
    }

    /* create a symlink to the file just created */
    err = IOFSL_INTERFACE(symlink)(trs1, trs2);
    if(err)
    {
        fprintf(stderr, "Could not symlink %s -> %s\n", trs1, trs2);
    }

    /* unlink the symlink and the file */
    err = IOFSL_INTERFACE(unlink)(trs1);
    if(err)
    {
        fprintf(stderr, "Could not unlink %s\n", trs1);
    }

    err = IOFSL_INTERFACE(unlink)(trs2);
    if(err)
    {
        fprintf(stderr, "Could not unlink %s\n", trs2);
    }

    return 0;
}

int link_test(char * base_path)
{
    int err = 0;
    int fd1 = 0;
    char trs1[4096];
    char trs2[4096];
  
    /* build the file names */
    sprintf(trs1, "%s/sysio.test.link.txt", base_path);
    sprintf(trs2, "%s/sysio.test.link.txt.sl", base_path);
 
    /* open and create the file */ 
    fd1 = IOFSL_INTERFACE(open)(trs1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s\n", trs1);
    }
   
    /* close the fd */ 
    err = close(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s\n", trs1);
    }

    /* create a symlink to the file just created */
    err = IOFSL_INTERFACE(link)(trs1, trs2);
    if(err)
    {
        fprintf(stderr, "Could not link %s -> %s\n", trs1, trs2);
    }

    /* unlink the symlink and the file */
    err = IOFSL_INTERFACE(unlink)(trs1);
    if(err)
    {
        fprintf(stderr, "Could not unlink %s\n", trs1);
    }

    err = IOFSL_INTERFACE(unlink)(trs2);
    if(err)
    {
        fprintf(stderr, "Could not unlink %s\n", trs2);
    }

    return 0;
}

int mkdir_rmdir_test(char * base_path)
{
    int err = 0;
    char trm1[4096];
    char trm2[4096];
    char trm3[4096];
    char trm4[4096];

    /* build the file names */
    sprintf(trm1, "%s/subdir", base_path);
    sprintf(trm2, "%s/subdir/1", base_path);
    sprintf(trm3, "%s/subdir/1/2", base_path);
    sprintf(trm4, "%s/subdir/1/2/3", base_path);

    /* mkdir tests */
    err = IOFSL_INTERFACE(mkdir)(trm1, 0755);
    if(err)
    {
        fprintf(stderr, "Could not create dir %s\n", trm1);
    }

    err = IOFSL_INTERFACE(mkdir)(trm2, 0755);
    if(err)
    {
        fprintf(stderr, "Could not create dir %s\n", trm2);
    }

    err = IOFSL_INTERFACE(mkdir)(trm3, 0755);
    if(err)
    {
        fprintf(stderr, "Could not create dir %s\n", trm3);
    }

    err = IOFSL_INTERFACE(mkdir)(trm4, 0755);
    if(err)
    {
        fprintf(stderr, "Could not create dir %s\n", trm4);
    }

    /* rmdir tests */
    err = IOFSL_INTERFACE(rmdir)(trm4);
    if(err)
    {
        fprintf(stderr, "Could not remove dir %s\n", trm4);
    }

    err = IOFSL_INTERFACE(rmdir)(trm3);
    if(err)
    {
        fprintf(stderr, "Could not remove dir %s\n", trm3);
    }

    err = IOFSL_INTERFACE(rmdir)(trm2);
    if(err)
    {
        fprintf(stderr, "Could not remove dir %s\n", trm2);
    }

    err = IOFSL_INTERFACE(rmdir)(trm1);
    if(err)
    {
        fprintf(stderr, "Could not remove dir %s\n", trm1);
    }

    return 0;
}

int stat_test(char * base_path)
{
    int err = 0;
    int fd1 = 0;
    char rbuffer[1024];
    char trrl1[4096];
    struct stat stbuf;

    /* build the file names */
    sprintf(trrl1, "%s/sysio.test.stat.txt", base_path);

    /* open and create the file */
    fd1 = IOFSL_INTERFACE(open)(trrl1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s\n", trrl1);
    }

    /* close the fd */
    err = close(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s\n", trrl1);
    }

    err = IOFSL_INTERFACE(stat)(trrl1, &stbuf);
    if(err == -1)
    {
        fprintf(stderr, "Could not stat %s\n", trrl1);
    }

    err = IOFSL_INTERFACE(unlink)(trrl1);
    if(err == -1)
    {
        fprintf(stderr, "Could not unlink %s\n", trrl1);
    }
 
    return 0;
}

int readlink_test(char * base_path)
{
    int err = 0;
    int fd1 = 0;
    char rbuffer[1024];
    char trrl1[4096];
    char trrl2[4096];

    /* build the file names */
    sprintf(trrl1, "%s/sysio.test.rl.txt", base_path);
    sprintf(trrl2, "%s/sysio.test.rl.txt.sl", base_path);

    /* open and create the file */
    fd1 = IOFSL_INTERFACE(open)(trrl1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s\n", trrl1);
    }

    /* close the fd */
    err = close(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s\n", trrl1);
    }

    /* create a symlink to the file just created */
    err = IOFSL_INTERFACE(symlink)(trrl1, trrl2);
    if(err)
    {
        fprintf(stderr, "Could not symlink %s -> %s\n", trrl1, trrl2);
    }

    /* readlink test */
    err = IOFSL_INTERFACE(readlink)(trrl2, rbuffer, 1024);
    if(err == -1)
    {
        fprintf(stderr, "Could not readlink %s\n", trrl2);
    }
    else
    {
        rbuffer[err] = '\0';
        fprintf(stderr, "readlink results for %s: %s\n", trrl2, rbuffer);
    }

    /* unlink the symlink and the file */
    err = IOFSL_INTERFACE(unlink)(trrl1);
    if(err)
    {
        fprintf(stderr, "Could not unlink %s\n", trrl1);
    }

    err = IOFSL_INTERFACE(unlink)(trrl2);
    if(err)
    {
        fprintf(stderr, "Could not unlink %s\n", trrl2);
    }

}

int write_read_truncate_test(char * base_path)
{
    /* write test */
    char writebuffer[1024];
    char readbuffer[1024];
    int err = 0;
    int fd1 = 0;
    char tf1[4096];
    struct stat stbuf;

    sprintf(tf1, "%s/rw-test-file.txt", base_path);

    /* setup the buffers */
    memset(writebuffer, 0, 1024);
    memset(readbuffer, 0, 1024);
    sprintf(writebuffer, "This is some test data! This is some test data! This is some test data! This is some test data! This is some test data!\n");
    fprintf(stderr, "write buffer contents len = %i: %s\n", strlen(writebuffer), writebuffer);

    fd1 = IOFSL_INTERFACE(open)(tf1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s\n", tf1);
    }
 
    /* write the buffer to the file */ 
    err = IOFSL_INTERFACE(write)(fd1, writebuffer, strlen(writebuffer));
    if(err == -1)
    {
        fprintf(stderr, "Could not write buffer to file %s\n", tf1);
    }

    err = IOFSL_INTERFACE(close)(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close file %s\n", tf1); 
    }

    fd1 = IOFSL_INTERFACE(open)(tf1, O_RDWR, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open file %s\n", tf1);
    }

    /* read the buffer from the file */ 
    err = IOFSL_INTERFACE(read)(fd1, readbuffer, strlen(writebuffer));
    if(err == -1)
    {
        fprintf(stderr, "Could not read from file %s\n", tf1);
    }
    else
    {
        fprintf(stderr, "file contents from read len = %i: %s\n", strlen(readbuffer), readbuffer);
        if(memcmp(writebuffer, readbuffer, strlen(writebuffer)) == 0)
        {
            fprintf(stderr, "read / write buffers match\n");
        }
        else
        {
            int len = strlen(writebuffer);
            int count = 0;
            for(count = 0 ; count < len ; count++)
            {
                if(readbuffer[count] != writebuffer[count])
                {
                    fprintf(stderr, "r[%i] = %c, w[%i] = %c\n", count, readbuffer[count], count, writebuffer[count]);
                }
            }
            fprintf(stderr, "read / write buffers do not match\n");
        }
    }

    int readoffs = strlen(writebuffer);
    memset(writebuffer, 0, 1024);
    sprintf(writebuffer, "More test data! More test data! More test data! More test data! More test data! More test data!\n");
    /* write the buffer to the file */
    err = IOFSL_INTERFACE(write)(fd1, writebuffer, strlen(writebuffer));
    err = IOFSL_INTERFACE(write)(fd1, writebuffer, strlen(writebuffer));
    if(err == -1)
    {
        fprintf(stderr, "Could not write buffer to file %s\n", tf1);
    }

    err = IOFSL_INTERFACE(close)(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close file %s\n", tf1);
    }

    fd1 = IOFSL_INTERFACE(open)(tf1, O_RDWR, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open file %s\n", tf1);
    }

    /* read the buffer from the file */
    err = IOFSL_INTERFACE(read)(fd1, readbuffer, readoffs + (2 * strlen(writebuffer)));
    if(err == -1)
    {
        fprintf(stderr, "Could not read from file %s\n", tf1);
    }
    else
    {
        fprintf(stderr, "file contents from read: %s\n", readbuffer);
        fprintf(stderr, "write buffer len = %i\n", strlen(writebuffer));
        fprintf(stderr, "read buffer len = %i\n", strlen(writebuffer));
        if(memcmp(writebuffer, &readbuffer[readoffs], strlen(writebuffer)) == 0)
        {
            fprintf(stderr, "read / write buffers match\n");
        }
        else
        {
            int len = strlen(writebuffer);
            int count = 0;
            for(count = 0 ; count < len ; count++)
            {
                if(readbuffer[readoffs + count] != writebuffer[count])
                {
                    fprintf(stderr, "r[%i] = %c, w[%i] = %c\n", count, readbuffer[readoffs + count], count, writebuffer[count]);
                }
            }
            len = strlen(readbuffer);
            count = 0;
            for(count = 0 ; count < len ; count++)
            {
                fprintf(stderr, "r[%i] = %c\n", count, readbuffer[count]);
            }
            fprintf(stderr, "read / write buffers do not match\n");
        }
    }

    err = IOFSL_INTERFACE(ftruncate)(fd1, 32);
    if(err == -1)
    {
        fprintf(stderr, "Could not truncate file %s\n", tf1);
    }
    else
    {
        err = IOFSL_INTERFACE(stat)(tf1, &stbuf);
        if(err == -1)
        {
            fprintf(stderr, "Could not stat %s\n", tf1);
        }
        else
        {
            if(stbuf.st_size == 32)
            {
                fprintf(stderr, "Could set the correct size with truncate for %s, %u == %u\n", tf1, 32, stbuf.st_size);
            }
            else
            {
                fprintf(stderr, "Could not set the correct size with truncate for %s, %u == %u\n", tf1, 32, stbuf.st_size);
            }
        }
    }
 

    err = IOFSL_INTERFACE(close)(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close file %s\n", tf1); 
    }

    /* unlink the file */
    //err = IOFSL_INTERFACE(unlink)(tf1);
    if(err)
    {
        fprintf(stderr, "Could not unlink file %s\n", tf1);
    }

    return 0;
}

int chmod_test(char * base_path)
{
    int err = 0;
    int fd1 = 0;
    char rbuffer[1024];
    char trrl1[4096];
    struct stat stbuf;

    /* build the file names */
    sprintf(trrl1, "%s/sysio.test.stat.txt", base_path);

    /* open and create the file */
    fd1 = IOFSL_INTERFACE(open)(trrl1, O_RDWR | O_CREAT, 0644);
    if(fd1 == -1)
    {
        fprintf(stderr, "Could not open %s\n", trrl1);
    }

    /* close the fd */
    err = close(fd1);
    if(err)
    {
        fprintf(stderr, "Could not close fd for %s\n", trrl1);
    }

    err = IOFSL_INTERFACE(chmod)(trrl1, 0755);
    if(err == -1)
    {
        fprintf(stderr, "Could not chmod %s\n", trrl1);
    }

    err = IOFSL_INTERFACE(stat)(trrl1, &stbuf);
    if(err == -1)
    {
        fprintf(stderr, "Could not stat %s\n", trrl1);
    }
    else
    {
        if((stbuf.st_mode & 0777) == 0755)
        {
            fprintf(stderr, "Could set the correct perms with chmod for %s, %o == 0755\n", trrl1, stbuf.st_mode & 0777);
        }
        else
        {
            fprintf(stderr, "Could not set the correct perms with chmod for %s, %o != 0755\n", trrl1, stbuf.st_mode & 0777);
        }
    }

    err = IOFSL_INTERFACE(unlink)(trrl1);
    if(err == -1)
    {
        fprintf(stderr, "Could not unlink %s\n", trrl1);
    }

    return 0;
}

int readdir_test(char * base_path)
{
    DIR *dirp;
    struct dirent *dp = NULL;

    if ((dirp = opendir(base_path)) == NULL) {
        fprintf(stderr, "could not open %s\n", base_path);
        return 0;
    }

    do
    {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL)
        {
            fprintf(stderr, "found dir ent = %s\n", dp->d_name);
        }
        else
        {
            fprintf(stderr, "dp == NULL\n");
        }
    }while (dp != NULL);


    if (errno != 0)
        perror("error reading directory");
    
    closedir(dirp);

    return 0;
}

int scandir_test(char * base_path)
{
    struct dirent **namelist;
    int n;

    n = scandir(base_path, &namelist, 0, alphasort);
    if (n < 0)
        perror("scandir");
    else {
        while (n--) {
            printf("%s\n", namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
    return 0;
}

int main(int argc, char * args[])
{
    /* check for proper cmd line args */
    if(argc < 2)
    {
        fprintf(stderr, "./posix-cunit <base path>");
        return -1;
    }

    _iofwd_sysio_startup();

    /* tests */
    open_close_unlink_test(args[1]);
    //rename_test(args[1]);
    //symlink_test(args[1]);
    //link_test(args[1]);
    //mkdir_rmdir_test(args[1]);
    //readlink_test(args[1]);
    //stat_test(args[1]);
    //chmod_test(args[1]);
    /*readdir_test(args[1]);*/
    /*scandir_test(args[1]);*/
    write_read_truncate_test(args[1]);

    fprintf(stderr, "%s : done\n", __func__);

    return 0;
}
