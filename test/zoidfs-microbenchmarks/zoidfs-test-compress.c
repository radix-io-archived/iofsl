#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

#include "zoidfs/zoidfs.h"

static double sfac = 0.0;

/* the kernel to compute the data */
double kernel(double x, double y)
{
    /* kernel */
    double data = -1.0 * sfac * ( cos(x) + cos(y) );

    /* if the cur value is less than 0, set it to 0 */
    if(data < 0.0)
    {
        data = 0.0;
    }

    return data;
}

int main(int argc, char * args[])
{
    /* kernel boundaries */
    double x_start = 0.0;
    double x_end = 0.0;
    double x_inc = 0.0; 
    double x_cur = 0.0;
    double y_start = 0.0;
    double y_end = 0.0;
    double y_inc = 0.0;
    double y_cur = 0.0;

    double zfs_domain = 0.0;
    double inc_amt = 0.0;

    /* local proc variables */
    int ret = 0;
    int my_rank = 0;
    int my_size = 0;
    double stime = 0.0;
    double etime = 0.0;
    double etime_avg = 0.0;
    int itr = 0;
    int max_itr = 0;

    zoidfs_handle_t handle;
    struct timeval now;
    zoidfs_sattr_t sattr;
    char * path = NULL;
    char * mode = NULL;
    int created = 0;

    double * data = NULL;

    MPI_Init(&argc, &args);

    /* setup the cmd line args */
    if(argc < 7)
    {
        fprintf(stderr, "incorrect args!\n");
        fprintf(stderr, "./zoidfs-test-compress SFAC DOMAIN_SIZE INC MODE MAXITR PATH\n");
        MPI_Finalize();
        return -1;
    }

    sfac = atof(args[1]);
    zfs_domain = atof(args[2]);
    inc_amt = atof(args[3]);
    mode = args[4];
    max_itr = atoi(args[5]);
    path = args[6];
   
    x_end = 2.0 * zfs_domain; 
    y_end = 2.0 * zfs_domain;
    x_inc = inc_amt;
    y_inc = inc_amt;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &my_size);

    int total_ops = ( (2.0 * zfs_domain * 2.0 * zfs_domain) / (inc_amt * inc_amt) ) / my_size;
    int cur_ops = 0;
    int local_ds = sizeof(double) * total_ops;

    memset(&sattr, 0, sizeof(sattr));

    /* set the attrs */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0644;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* load the I/O description */
    size_t mem_count = 0;
    void ** mem_starts = NULL;
    size_t * mem_sizes = NULL;
    zoidfs_file_ofs_t * file_starts = NULL;
    zoidfs_file_size_t * file_sizes = NULL;
    
    /* row decomp */
    if(strcmp(mode, "R") == 0)
    {
        /* load the I/O description */
        mem_count = 1;
        mem_starts = malloc(mem_count * sizeof(void *));
        mem_sizes = (size_t *)malloc(mem_count * sizeof(size_t));
        file_starts = (zoidfs_file_ofs_t *)malloc(mem_count * sizeof(zoidfs_file_ofs_t));
        file_sizes = (zoidfs_file_size_t *)malloc(mem_count * sizeof(zoidfs_file_size_t));

        double my_y_start = (y_end - y_start) / my_size * my_rank;
        double my_y_end = ((y_end - y_start) / my_size * (my_rank + 1));
        int j = 0;

        /* create the data buffer */
        data = (double *)malloc(sizeof(double) * total_ops);

        /* iterate over the y range */    
        for(y_cur = my_y_start ; y_cur < my_y_end ; y_cur += y_inc)
        {
            /* iterate over the x range */    
            for(x_cur = x_start ; x_cur < x_end ; x_cur += x_inc)
            {
                /* kernel */
                data[j] = kernel(x_cur, y_cur);
                cur_ops++;
            }
        }

        mem_starts[0] = data;
        mem_sizes[0] = sizeof(double) * total_ops;
        file_starts[0] = my_rank * sizeof(double) * total_ops;
        file_sizes[0] = sizeof(double) * total_ops;

        //fprintf(stderr, "rank %i: mem_starts = %p, mem_sizes = %llu, file_starts = %llu, file_sizes = %llu\n", my_rank, mem_starts[0], mem_sizes[0], file_starts[0], file_sizes[0]);
    }
    /* column decomp */
    else if(strcmp(mode, "C") == 0)
    {
        /* load the I/O description */
        mem_count = (y_end - y_start) / inc_amt;
        mem_starts = malloc(mem_count * sizeof(void *));
        mem_sizes = (size_t *)malloc(mem_count * sizeof(size_t));
        file_starts = (zoidfs_file_ofs_t *)malloc(mem_count * sizeof(zoidfs_file_ofs_t));
        file_sizes = (zoidfs_file_size_t *)malloc(mem_count * sizeof(zoidfs_file_size_t));

        double my_x_start = (x_end - x_start) / my_size * my_rank;
        double my_x_end = ((x_end - x_start) / my_size * (my_rank + 1));
        int rl = ((x_end - x_start) / inc_amt);
        int i = 0;
        int j = 0;

        /* create the data buffer */
        data = (double *)malloc(sizeof(double) * total_ops);

        /* iterate over the y range */    
        for(y_cur = y_start ; y_cur < y_end ; y_cur += y_inc, i++)
        {
            mem_starts[i] = &data[j];
            mem_sizes[i] = sizeof(double) * (int)((my_x_end - my_x_start) / inc_amt);
            file_starts[i] = (sizeof(double) * rl * i) + (sizeof(double) * (my_x_start - x_start) / inc_amt);
            file_sizes[i] = sizeof(double) * (int)((my_x_end - my_x_start) / inc_amt);
            /* iterate over the x range */    
            for(x_cur = my_x_start ; x_cur < my_x_end ; x_cur += x_inc, j++)
            {
                /* kernel */
                data[j] = kernel(x_cur, y_cur);
            }
        }

        mem_starts[0] = data;
        mem_sizes[0] = sizeof(double) * total_ops;
        file_starts[0] = my_rank * sizeof(double) * total_ops;
        file_sizes[0] = sizeof(double) * total_ops;
    }
    /* tile decomp */
    else if(strcmp(mode, "T") == 0)
    {
        int dim = (int)sqrt(my_size);
        double v = (x_end - x_start) / dim;
        int rl = (x_end - x_start) / inc_amt;
        int rpos = my_rank / dim;
        int cpos = my_rank % dim;

        double my_x_start = cpos * v;
        double my_x_end = v * (cpos + 1);
        double my_y_start = v * rpos;
        double my_y_end = v * (rpos + 1);

        int i = 0;
        int j = 0;

        /* load the I/O description */
        mem_count = v / inc_amt;
        mem_starts = malloc(mem_count * sizeof(void *));
        mem_sizes = (size_t *)malloc(mem_count * sizeof(size_t));
        file_starts = (zoidfs_file_ofs_t *)malloc(mem_count * sizeof(zoidfs_file_ofs_t));
        file_sizes = (zoidfs_file_size_t *)malloc(mem_count * sizeof(zoidfs_file_size_t));

        /* create the data buffer */
        data = (double *)malloc(sizeof(double) * total_ops);

        for(y_cur = my_y_start ; y_cur < my_y_end ; y_cur += y_inc, i++)
        {
            mem_starts[i] = &data[j];
            mem_sizes[i] = sizeof(double) * mem_count;
            file_starts[i] = (sizeof(double) * rl * ((rpos * rl / dim) + i)) + (sizeof(double) * mem_count * cpos);
            file_sizes[i] = sizeof(double) * mem_count;
    
            //fprintf(stderr, "rank = %i, rl = %i, rpos = %i, cpos = %i, mem_starts = %p, mem_sizes = %lu, file_starts = %lu, file_sizes = %lu\n", my_rank, rl, rpos, cpos, mem_starts[i], mem_sizes[i], file_starts[i], file_sizes[i]);
            /* iterate over the x range */    
            for(x_cur = my_x_start ; x_cur < my_x_end ; x_cur += x_inc, j++)
            {
                /* kernel */
                data[j] = kernel(x_cur, y_cur);
            }
        }
    }

    /* init the zoidfs interface */
    zoidfs_init();

    /* setup the file handle */
    if(my_rank == 0)
    {
        ret = zoidfs_create(NULL, NULL, path, &sattr, &handle, &created, NULL);
        if(ret != ZFS_OK)
        {
            fprintf(stderr, "error: could not create file, path = %s, err = %i\n", path, ret);
        }

        /* wait for the other procs */
        MPI_Barrier(MPI_COMM_WORLD);
    }
    else
    {
        /* wait for file creation */
        MPI_Barrier(MPI_COMM_WORLD);

        /* lookup the file */
        ret = zoidfs_lookup(NULL, NULL, path, &handle, NULL);
        if(ret != ZFS_OK)
        {
            fprintf(stderr, "error: could not lookup file, path = %s, err = %i\n", path, ret);
        }
    }

    /* issue the IO */
    for(itr = 0 ; itr < max_itr ; itr++)
    {
        /* wait for all procs before we issue the write */
        MPI_Barrier(MPI_COMM_WORLD);
        stime = MPI_Wtime();
    
        ret = zoidfs_write(&handle, mem_count, (const void **)mem_starts, mem_sizes, mem_count, file_starts, file_sizes, NULL);
        if(ret != ZFS_OK)
        {
            fprintf(stderr, "error: could not write data, err = %i\n", ret);
        }
        etime += (MPI_Wtime() - stime);
    }

    /* compute the avg elapsed time for a write */
    etime_avg = etime / max_itr;

    double max_time = 0.0;
    double min_time = 0.0;
    double sum_time = 0.0;

    /* collect the results */
    MPI_Reduce(&etime_avg, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    MPI_Reduce(&etime_avg, &min_time, 1, MPI_DOUBLE, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&etime_avg, &sum_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(my_rank == 0)
    {
        int ds = ((x_end - x_start) / inc_amt) * ((y_end - y_start) / inc_amt) * sizeof(double);
        fprintf(stdout, "result: time min = %f, max = %f, avg = %f\n", min_time, max_time, sum_time / my_size);
        fprintf(stdout, "result: localds = %i, ds = %i, tput min = %f, max = %f, avg = %f\n", local_ds, ds, local_ds / min_time / (1024 * 1024), local_ds / max_time / (1024 * 1024), ds / (sum_time / my_size) / (1024 * 1024));
    }

    /* cleanup the data buffer */
    free(data);
    free(mem_starts);
    free(mem_sizes);
    free(file_starts);
    free(file_sizes);
  
    /* remove the file */
    if(my_rank == 0)
    {
        //ret = zoidfs_remove(NULL, NULL, path, NULL, NULL);
        if(ret != ZFS_OK)
        {
            fprintf(stderr, "error: could not remove file = %s, err = %i\n", path, ret);
        }
    }

    /* shutdown the zoidfs interface */
    zoidfs_finalize();

    /* shutdown the mpi interface */
    MPI_Finalize();
 
    return 0;
}
