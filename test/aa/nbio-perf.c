#include <mpi.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/hints/zoidfs-hints.h"

#define ENABLE_NB_SERVER_MODE
//#define ENABLE_NB_COMMIT_BEFORE_TIMER

void run(char * file_path, int rank, int size, unsigned int sleep_time, size_t buffer_size, int num_itr)
{
    zoidfs_handle_t handle;
    zoidfs_sattr_t sattr;
    zoidfs_op_hint_t op_hint;
    struct timeval now;
    int created = 0;
    int i = 0;
    char * buffer = (char *)malloc(sizeof(char) * buffer_size);
    double start = 0;
    double stop = 0;
    double elapsed = 0;
    double t_max = 0;
    double t_min = 0;
    double t_sum = 0;
    int ret = 0;

    zoidfs_init();

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

    /* create or lookup the file */
    ret = zoidfs_create(NULL, NULL, file_path, &sattr, &handle, &created, NULL);
    if(ret != ZFS_OK)
        MPI_Abort(MPI_COMM_WORLD, 1);

    zoidfs_hint_create(&op_hint);
    MPI_Barrier(MPI_COMM_WORLD);

    start = MPI_Wtime();

    /* all ranks write to the file */
    for(i = 0 ; i < num_itr ; i++)
    {
        int memcount = 1;
        size_t memsize = buffer_size;
        const void * memstart = buffer;
        int filecount = 1;
        size_t filesize = buffer_size;
        size_t filestart = (rank * buffer_size * num_itr) +
            (buffer_size * i);

#ifdef ENABLE_NB_SERVER_MODE
        zoidfs_hint_set(op_hint, ZOIDFS_NONBLOCK_SERVER_IO,
                ZOIDFS_HINT_ENABLED, 0);
#ifdef DROP_NBIO_REQUESTS
        zoidfs_hint_set(op_hint, ZOIDFS_NONBLOCK_SERVER_DROP_IO,
                ZOIDFS_HINT_ENABLED, 0);
#endif
#endif
        /* sleep */
        usleep(sleep_time * 1000); /* in ms */

        /* write the data */
        ret = zoidfs_write(&handle, memcount, &memstart, &memsize, filecount,
                &filestart, &filesize, &op_hint);
        fprintf(stderr, "%s:%i zoidfs_write done, filestart = %lu\n",
                __func__, __LINE__, filestart);
        if(ret != ZFS_OK)
            MPI_Abort(MPI_COMM_WORLD, 1);

#ifdef ENABLE_NB_SERVER_MODE
        zoidfs_hint_delete_all(op_hint);
#endif
    }

#ifdef ENABLE_NB_SERVER_MODE
    if(rank == 0)
    {
#ifdef ENABLE_NB_COMMIT_BEFORE_TIMER
        zoidfs_hint_set(op_hint, ZOIDFS_NONBLOCK_SERVER_IO,
                ZOIDFS_HINT_ENABLED, 0);
        ret = zoidfs_commit(&handle, &op_hint);
        if(ret != ZFS_OK)
            MPI_Abort(MPI_COMM_WORLD, 1);
#endif
    }
#endif

    stop = MPI_Wtime();
    elapsed = stop - start;

    MPI_Reduce(&elapsed, &t_sum, 1, MPI_DOUBLE, MPI_SUM, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&elapsed, &t_min, 1, MPI_DOUBLE, MPI_MIN, 0,
            MPI_COMM_WORLD);

    if(rank == 0)
    {
        fprintf(stderr, "ni %i bs %lu st %u size %i max %lf min %lf avg %lf\n",
            num_itr, buffer_size, sleep_time, size, t_max, t_min, t_sum / (1.0 * size));
    }

#ifdef ENABLE_NB_SERVER_MODE
    if(rank == 0)
    {
#ifndef ENABLE_NB_COMMIT_BEFORE_TIMER
        zoidfs_hint_set(op_hint, ZOIDFS_NONBLOCK_SERVER_IO,
                ZOIDFS_HINT_ENABLED, 0);
        ret = zoidfs_commit(&handle, &op_hint);
        if(ret != ZFS_OK)
            MPI_Abort(MPI_COMM_WORLD, 1);
#endif
    }
#endif

    stop = MPI_Wtime();
    elapsed = stop - start;
    /* rank 0 removes the file */
    if(rank == 0)
    {
        /* remove the file */
        ret = zoidfs_remove(NULL, NULL, file_path, NULL, NULL);
        if(ret != ZFS_OK)
            MPI_Abort(MPI_COMM_WORLD, 1);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    zoidfs_hint_free(&op_hint);
    free(buffer);

    zoidfs_finalize();

}

int main(int argc, char ** args)
{
    int rank = 0;
    int size = 0;
    char * file_path = NULL;
    unsigned int sleep_time = 0;
    size_t buffer_size = 0;
    int num_itr = 0;

    /* init */
    MPI_Init(&argc, &args);

    /* get the cur proc rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* get cmd line args */
    file_path = args[1];
    sleep_time = atoi(args[2]);
    buffer_size = atoi(args[3]);
    num_itr = atoi(args[4]);

#if 0
    if(rank == 0)
    {
        fprintf(stderr, "file = %s\n", file_path);
        fprintf(stderr, "sleep_time = %u\n", sleep_time);
        fprintf(stderr, "buffer_size = %lu\n", buffer_size);
        fprintf(stderr, "num_itr = %i\n", num_itr);
        fprintf(stderr, "size = %i\n", size);
    }
    MPI_Barrier(MPI_COMM_WORLD);
#endif

    /* run the test */
    run(file_path, rank, size, sleep_time, buffer_size, num_itr);

    /* cleanup */
    MPI_Finalize();

    return 0;
}
