#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>

#include <mpi.h>
#include "zoidfs/zoidfs.h"
#include "zoidfs/hints/zoidfs-hints.h"

/* how many buffers will each rank write */
#define BUFS_PER_RANK 16 

/* enable or disable AA mode */
#if 1
#define ENABLE_AA_MODE
#endif

//#define ENABLE_NB_SERVER_MODE

/*
* simple test program for atomic append mode
* 
*   file is divided into contogous blocks for each
*   process. In normal mode, eacho process writes only
*   into its contigous block. In AA mode, the client
*   file offset data is ignored by the server and the
*   data is appended to the end of the file.
*/
int main(int argc, char **argv)
{
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_handle_t handle;
    zoidfs_handle_t basedir_handle;
    char filename[] = "aa-write-test";
    int i, numproc, rank, ret;
    zoidfs_op_hint_t op_hint;

    /* init MPI, get the rank, get the size of comm world */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* check args */
    if(argc < 2)
    {
        fprintf(stderr, "%s:%i incorrect args\n", __func__, __LINE__);
        fprintf(stderr, "%s:%i should be similar to 'zoidfs-aa-write /dir/to/store/file/in'\n", __func__, __LINE__);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    /* start the zoidfs / iofsl interface */
    ret = zoidfs_init();
    if(ret != ZFS_OK)
    {
        fprintf(stderr, "%s:%i zoidfs_init error\n", __func__, __LINE__);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    /* lookup the directory on the cmd line, verify that it is valid */
    ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle,
            ZOIDFS_NO_OP_HINT);
    if(ret != ZFS_OK)
    {
        fprintf(stderr, "%s:%i zoidfs_lookup error\n", __func__, __LINE__);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    /* Set the attributes */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0644;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* rank 0 will create the file. if the file exists, if will delete it
       and recreate an empty file */
    if(rank == 0)
    {
        int created = 0;

        /* create the file */
        ret = zoidfs_create(&basedir_handle, filename, NULL, &sattr,
            &handle, &created, ZOIDFS_NO_OP_HINT);
        if(ret != ZFS_OK)
        {
            fprintf(stderr, "%s:%i create file error\n", __func__,
                    __LINE__);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }

        /* file was not created, try to remove it */
        if(!created)
        {
            /* remove the file */
            ret = zoidfs_remove(&basedir_handle, filename, NULL, NULL,
                    ZOIDFS_NO_OP_HINT);
            if(ret != ZFS_OK)
            {
                fprintf(stderr, "%s:%i could not remove existing file\n",
                        __func__, __LINE__);
                MPI_Abort(MPI_COMM_WORLD, -1);
            }
           
            /* recreate the file */ 
            created = 0;
            ret = zoidfs_create(&basedir_handle, filename, NULL, &sattr,
                &handle, &created, ZOIDFS_NO_OP_HINT);
            if(ret != ZFS_OK)
            {
                fprintf(stderr, "%s:%i create file error\n", __func__,
                        __LINE__);
                MPI_Abort(MPI_COMM_WORLD, -1);
            }
            /* could not recreate the file, abort */
            if(!created)
            {
                fprintf(stderr, "%s:%i could not create file\n", __func__,
                        __LINE__);
                MPI_Abort(MPI_COMM_WORLD, -1);
            }
        }
        /* meet with all other ranks waiting for the file to be created */
        MPI_Barrier(MPI_COMM_WORLD);
    }
    /* all non-zero ranks will lookup the file */
    else
    {
        /* wait for the file to be created */
        MPI_Barrier(MPI_COMM_WORLD);

        /* lookup the file handle */
        ret = zoidfs_lookup(&basedir_handle, filename, NULL, &handle,
                ZOIDFS_NO_OP_HINT);
        if(ret != ZFS_OK)
        {
            fprintf(stderr, "%s:%i file lookup error\n", __func__,
                    __LINE__);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
    }

    /* init the hint */
    zoidfs_hint_create(&op_hint);

    /* write params */
    zoidfs_file_size_t fsize = (4 * 1024 * 1024); 
    size_t buffer_size = (4 * 1024 * 1024); 
    zoidfs_file_ofs_t fofs = BUFS_PER_RANK * rank * buffer_size; 
    void * buffer = malloc(sizeof(char) * buffer_size);

    /* start the write iterations for each client */
    for( i = 0 ; i < BUFS_PER_RANK ; i++)
    {
        long unsigned newoff = 0;
       
#ifdef ENABLE_AA_MODE 
        int flag = 0;
        /* set the atomic append hint */
        zoidfs_hint_set(op_hint, ZOIDFS_ATOMIC_APPEND,
                ZOIDFS_HINT_ENABLED, 0);
#endif
#ifdef ENABLE_NB_SERVER_MODE
        zoidfs_hint_set(op_hint, ZOIDFS_NONBLOCK_SERVER_IO,
                ZOIDFS_HINT_ENABLED, 0);
#endif

        /* issue a write with the atomic append hint */
        ret = zoidfs_write(&handle, 1, (const void **)&buffer, &buffer_size,
                1, &fofs, &fsize, &op_hint); 
        if(ret != ZFS_OK)
        {
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
       
#ifdef ENABLE_AA_MODE 
        /* length defined in zoidfs-hints.h */
        char value[ZOIDFS_ATOMIC_APPEND_OFFSET_MAX_BYTES];

        /* get the atomic append offset */ 
        zoidfs_hint_get(op_hint, ZOIDFS_ATOMIC_APPEND_OFFSET,
                ZOIDFS_ATOMIC_APPEND_OFFSET_MAX_BYTES, value, &flag);
        
        newoff = (long unsigned)atol(value);
#else
        newoff = fofs;
#endif

        /* output file write info... where did the client specify the
           file write (old offset) and where write exists in the
           file (new offset) */
        fprintf(stderr, "%s:%i rank = %i, orig offset = %lu, append offset = %lu\n",
                __func__, __LINE__, rank, fofs, newoff);
        fofs += buffer_size; 

#ifdef ENABLE_AA_MODE 
        /* delete the hints */
        zoidfs_hint_delete_all(op_hint);
#endif
    }
 
    /* free the hint */
    zoidfs_hint_free(&op_hint);

    /* free the write buffer */
    free(buffer);

    MPI_Barrier(MPI_COMM_WORLD);

    /* rank 0 removes the file */
    if(rank == 0)
    {
        /* remove the file */ 
        ret = zoidfs_remove(&basedir_handle, filename, NULL, NULL,
                ZOIDFS_NO_OP_HINT);
        if(ret != ZFS_OK)
        {
            fprintf(stderr, "%s:%i zoidfs_remove error\n", __func__,
                    __LINE__);
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
    }

    /* meetup for shutdown... not really needed */
    MPI_Barrier(MPI_COMM_WORLD);
   
    /* shutdown zoidfs / iofsl on client side */ 
    zoidfs_finalize();
   
    /* shutdown MPI */ 
    MPI_Finalize();

    /* all done ... */
    return 0;
}
