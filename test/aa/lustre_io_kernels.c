#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <lustre/lustre_user.h>
#include <getopt.h>

/* Test various IO kernels on Lustre */

#define FILENAMELEN 8192
#define HOSTNAMELEN 128
char FILENAME[8192]; // "/scratch/copej/data/satest.data"

static int enable_grouplocks = 0;
static int enable_trace = 0;
static int enable_verbose = 0;
static int enable_mode = 0;
static int global_corespernode = 12; /* def 12 cores */
static int64_t global_numreps = 16; /* def 16 reps */
static int64_t global_buffersize = 4*1024*1024; /* def 4MB */

static pthread_mutex_t offset_mutex = PTHREAD_MUTEX_INITIALIZER;
static int64_t global_offset = 0;
static int64_t global_sw = 0;

void lustre_lock(int fd)
{
    int ret = 0;
    int gid = 7;

    if(enable_grouplocks)
    {
        ret = ioctl(fd, LL_IOC_GROUP_LOCK, gid);
        if(ret != 0)
        {
            fprintf(stderr, "%s:%i ERROR: could not acquire lustre group lock (gid = %i)\n", __func__, __LINE__, gid);
        }
    }

    /* wait for everyone to lock */
    MPI_Barrier(MPI_COMM_WORLD);
}

void lustre_unlock(int fd)
{
    int ret = 0;
    int gid = 7;

    /* wait for everyone to unlock */
    MPI_Barrier(MPI_COMM_WORLD);

    if(enable_grouplocks)
    {
        ret = ioctl(fd, LL_IOC_GROUP_UNLOCK, gid);
        if(ret != 0)
        {
            fprintf(stderr, "%s:%i ERROR: could not release lustre group lock (gid = %i)\n", __func__, __LINE__, gid);
        }
    }
}

void * gaa_global_offset_thread(void * args)
{
    int64_t i = 0;
    int64_t ns = *((int64_t *)args);
    int64_t in_args[2];
    int64_t out_args[1];
    MPI_Status status;
    int64_t target = 0;

    for( i = 0 ; i < ns ; i++ )
    {
        int64_t local_size = 0;
        MPI_Recv(&in_args[0], 2, MPI_LONG_LONG_INT, MPI_ANY_SOURCE, 0,
                MPI_COMM_WORLD, &status);
        target = in_args[0];
        local_size = in_args[1];
        
        pthread_mutex_lock(&offset_mutex);
        out_args[0] = global_offset;
        global_offset += global_sw;
        pthread_mutex_unlock(&offset_mutex);

        MPI_Send(&out_args[0], 1, MPI_LONG_LONG_INT, target, 0, MPI_COMM_WORLD);
    }

    pthread_exit(NULL);

    return NULL;
}

int64_t gaa_get_local_offset(int64_t size)
{
    int64_t off = 0;

    pthread_mutex_lock(&offset_mutex);
    off = global_offset;
    global_offset += global_sw;
    pthread_mutex_unlock(&offset_mutex);

    return off;
}

void * global_offset_thread(void * args)
{
    int64_t i = 0;
    int64_t ns = *((int64_t *)args);
    int64_t in_args[2];
    int64_t out_args[1];
    MPI_Status status;
    int64_t target = 0;

    for( i = 0 ; i < ns ; i++ )
    {
        int64_t local_size = 0;
        MPI_Recv(&in_args[0], 2, MPI_LONG_LONG_INT, MPI_ANY_SOURCE, 0,
                MPI_COMM_WORLD, &status);
        target = in_args[0];
        local_size = in_args[1];
        
        pthread_mutex_lock(&offset_mutex);
        out_args[0] = global_offset;
        global_offset += local_size;
        pthread_mutex_unlock(&offset_mutex);

        MPI_Send(&out_args[0], 1, MPI_LONG_LONG_INT, target, 0, MPI_COMM_WORLD);
    }

    pthread_exit(NULL);

    return NULL;
}

int64_t get_local_offset(int64_t size)
{
    int64_t off = 0;

    pthread_mutex_lock(&offset_mutex);
    off = global_offset;
    global_offset += size;
    pthread_mutex_unlock(&offset_mutex);

    return off;
}

void run_io_test_aaost(const int64_t size, const int id, const int64_t numid, const int64_t numrep)
{
    /* setup */
    int64_t i = 0;
    int64_t offset = 0;
    void * buffer = malloc(sizeof(char) * size);
    if(id == 0)
    {
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int64_t fd = open(FILENAME, O_RDWR | O_CREAT, 0644);
    double t_start = 0.0;
    double t_stop = 0.0;
    double t_elapsed = 0.0;
    double sum_t_elapsed = 0.0;
    double t_min = 0.0;
    double t_max = 0.0;
    pthread_t thread;

    lustre_lock(fd);

    int64_t * ns = (int64_t *)malloc(sizeof(int64_t));
    *ns = (numid - 1) * numrep;

    if(id == 0)
    {
        pthread_create(&thread, NULL, global_offset_thread, ns);
    }

    /* wait for everyone to open the file */
    MPI_Barrier(MPI_COMM_WORLD);

    t_start = MPI_Wtime();
    for(i = 0 ; i < numrep ; i++)
    {
        if(id == 0)
        {
            offset = get_local_offset(size);
        }
        else
        {
            MPI_Status status;
            int64_t in_arg[2];
            int64_t out_arg[1];

            in_arg[0] = id;
            in_arg[1] = size;

            MPI_Send(&in_arg[0], 2, MPI_LONG_LONG_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Recv(&out_arg[0], 1, MPI_LONG_LONG_INT, 0, 0, MPI_COMM_WORLD, &status);

            offset = out_arg[0];
        }
        if(enable_trace)
        {
            fprintf(stderr, "rank %i: i = %lli, offset = %lli, size = %lli\n", id, i,
                offset, size);
        }
        pwrite(fd, buffer, size, offset);
    }
    lustre_unlock(fd);
    close(fd);
    MPI_Barrier(MPI_COMM_WORLD);
    if(id == 0)
    {
        struct stat stat_buf;
        stat(FILENAME, &stat_buf);
        if(stat_buf.st_size != (int64_t)(numid * numrep * size))
        {
            fprintf(stderr, "%s:%i ERROR: unexpected file size\n", __func__,
                    __LINE__);
            fprintf(stderr, "%s:%i actual: %lli, expected = %lli\n", __func__,
                    __LINE__, stat_buf.st_size, (int64_t)(numid * numrep *
                        size));
        }
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    t_stop = MPI_Wtime();
    t_elapsed = (t_stop - t_start);

    /* report results */
    MPI_Reduce(&t_elapsed, &sum_t_elapsed, 1, MPI_DOUBLE, MPI_SUM, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_min, 1, MPI_DOUBLE, MPI_MIN, 0,
            MPI_COMM_WORLD);

    if(id == 0)
    {
        fprintf(stderr, "aa results: %lf\n", (t_max));
        pthread_join(thread, NULL);
    }

    /* cleanup */
    free(buffer);
    free(ns);
}

void run_io_test_gaaost(const int64_t size, const int id, const int64_t numid, const int64_t numrep)
{
    /* setup */
    int64_t group = id / global_corespernode;
    int64_t i = 0;
    int64_t offset = 0;
    void * buffer = malloc(sizeof(char) * size);
    if(id == 0)
    {
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int64_t fd = open(FILENAME, O_RDWR | O_CREAT, 0644);
    double t_start = 0.0;
    double t_stop = 0.0;
    double t_elapsed = 0.0;
    double sum_t_elapsed = 0.0;
    double t_min = 0.0;
    double t_max = 0.0;
    pthread_t thread;
    int64_t * ns = (int64_t *)malloc(sizeof(int64_t));

    lustre_lock(fd);

    *ns = (global_corespernode - 1) * numrep;

    if(id % global_corespernode == 0)
    {
        global_sw = size * (numid / global_corespernode);
        global_offset = group * size;
        pthread_create(&thread, NULL, gaa_global_offset_thread, ns);
    }

    /* wait for everyone to open the file */
    MPI_Barrier(MPI_COMM_WORLD);

    t_start = MPI_Wtime();
    for(i = 0 ; i < numrep ; i++)
    {
#if 1
        if(id % global_corespernode == 0)
        {
            offset = gaa_get_local_offset(size);
        }
        else
        {
            MPI_Status status;
            int64_t in_arg[2];
            int64_t out_arg[1];

            in_arg[0] = id;
            in_arg[1] = size;

            MPI_Send(&in_arg[0], 2, MPI_LONG_LONG_INT, group * global_corespernode, 0, MPI_COMM_WORLD);
            MPI_Recv(&out_arg[0], 1, MPI_LONG_LONG_INT, group * global_corespernode, 0, MPI_COMM_WORLD, &status);

            offset = out_arg[0];
        }
        if(enable_trace)
        {
            fprintf(stderr, "rank %i: i = %lli, offset = %lli, size = %lli\n", id, i,
                offset, size); 
        }
        pwrite(fd, buffer, size, offset);
#else
        offset = gaa_get_local_offset(size);
        pwrite(fd, buffer, size, offset);
#endif
    }
    lustre_unlock(fd);
    close(fd);
    MPI_Barrier(MPI_COMM_WORLD);
    if(id == 0)
    {
        struct stat stat_buf;
        stat(FILENAME, &stat_buf);
        if(stat_buf.st_size != (int64_t)(numid * numrep * size))
        {
            fprintf(stderr, "%s:%i ERROR: unexpected file size\n", __func__,
                    __LINE__);
            fprintf(stderr, "%s:%i actual: %lli, expected = %lli\n", __func__,
                    __LINE__, stat_buf.st_size, (int64_t)(numid * numrep *
                        size));
        }
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    t_stop = MPI_Wtime();
    t_elapsed = (t_stop - t_start);

    /* report results */
    MPI_Reduce(&t_elapsed, &sum_t_elapsed, 1, MPI_DOUBLE, MPI_SUM, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_min, 1, MPI_DOUBLE, MPI_MIN, 0,
            MPI_COMM_WORLD);

    if(id == 0)
    {
        fprintf(stderr, "gaa results: %lf\n", (t_max));
        pthread_join(thread, NULL);
    }

    /* cleanup */
    free(buffer);
    free(ns);
}

/* round robin through the osts */
void run_io_test_rrost(const int64_t size, const int id, const int64_t numid, const int64_t numrep)
{
    /* setup */
    int64_t i = 0;
    int64_t offset = 0;
    void * buffer = malloc(sizeof(char) * size);
    if(id == 0)
    {
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int64_t fd = open(FILENAME, O_RDWR | O_CREAT, 0644);
    double t_start = 0.0;
    double t_stop = 0.0;
    double t_elapsed = 0.0;
    double sum_t_elapsed = 0.0;
    double t_min = 0.0;
    double t_max = 0.0;

    lustre_lock(fd);

    /* wait for everyone to open the file */
    MPI_Barrier(MPI_COMM_WORLD);

    t_start = MPI_Wtime();
    for(i = 0 ; i < numrep ; i++)
    {
        if(enable_trace)
        {
            fprintf(stderr, "rank %i: i = %lli, offset = %lli, size = %lli\n", id, i,
                offset + (size * ((id + i) % numid)), size); 
        }
        pwrite(fd, buffer, size, offset + (size * ((id + i) % numid)));
        offset += (size * numid);
    }
    lustre_unlock(fd);
    close(fd);
    MPI_Barrier(MPI_COMM_WORLD);
    if(id == 0)
    {
        struct stat stat_buf;
        stat(FILENAME, &stat_buf);
        if(stat_buf.st_size != (int64_t)(numid * numrep * size))
        {
            fprintf(stderr, "%s:%i ERROR: unexpected file size\n", __func__,
                    __LINE__);
            fprintf(stderr, "%s:%i actual: %lli, expected = %lli\n", __func__,
                    __LINE__, stat_buf.st_size, (int64_t)(numid * numrep *
                        size));
        }
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    t_stop = MPI_Wtime();
    t_elapsed = (t_stop - t_start);

    /* report results */
    MPI_Reduce(&t_elapsed, &sum_t_elapsed, 1, MPI_DOUBLE, MPI_SUM, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_min, 1, MPI_DOUBLE, MPI_MIN, 0,
            MPI_COMM_WORLD);

    if(id == 0)
    {
        fprintf(stderr, "rr results: %lf\n", t_max);
    }

    /* cleanup */
    free(buffer);
}

/* one writer per OST */
void run_io_test_uniqost(const int64_t size, const int id, const int64_t numid, const int64_t numrep)
{
    /* setup */
    int64_t i = 0;
    int64_t offset = 0;
    void * buffer = malloc(sizeof(char) * size);
    if(id == 0)
    {
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int64_t fd = open(FILENAME, O_RDWR | O_CREAT, 0644);
    double t_start = 0.0;
    double t_stop = 0.0;
    double t_elapsed = 0.0;
    double sum_t_elapsed = 0.0;
    double t_min = 0.0;
    double t_max = 0.0;

    lustre_lock(fd);

    /* wait for everyone to open the file */
    MPI_Barrier(MPI_COMM_WORLD);

    t_start = MPI_Wtime();
    for(i = 0 ; i < numrep ; i++)
    {
        if(enable_trace)
        {
            fprintf(stderr, "rank %i: i = %lli, offset = %lli, size = %lli\n", id, i, offset + (size
                * id), size);
        }
        pwrite(fd, buffer, size, offset + (size * id));
        offset += (size * numid);
    }
    lustre_unlock(fd);
    close(fd);
    MPI_Barrier(MPI_COMM_WORLD);
    if(id == 0)
    {
        struct stat stat_buf;
        stat(FILENAME, &stat_buf);
        if(stat_buf.st_size != (int64_t)(numid * numrep * size))
        {
            fprintf(stderr, "%s:%i ERROR: unexpected file size\n", __func__,
                    __LINE__);
            fprintf(stderr, "%s:%i actual: %lli, expected = %lli\n", __func__,
                    __LINE__, stat_buf.st_size, (int64_t)(numid * numrep *
                        size));
        }
        unlink(FILENAME);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    t_stop = MPI_Wtime();
    t_elapsed = (t_stop - t_start);

    /* report results */
    MPI_Reduce(&t_elapsed, &sum_t_elapsed, 1, MPI_DOUBLE, MPI_SUM, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_min, 1, MPI_DOUBLE, MPI_MIN, 0,
            MPI_COMM_WORLD);

    if(id == 0)
    {
        fprintf(stderr, "uniq results: %lf\n", t_max);
    }

    free(buffer);
}

/* one writer per OST */
void run_io_test_nfile(const int64_t size, const int id, const int64_t numid, const int64_t numrep)
{
    /* setup */
    int64_t i = 0;
    int64_t group = id / global_corespernode;
    int64_t offset = 0;
    char nfile_filename[1024];
    char nfile_dirname[1024];

    sprintf(nfile_filename, "%s.%lli/file", FILENAME, group);
    sprintf(nfile_dirname, "%s.%lli", FILENAME, group);
    
    void * buffer = malloc(sizeof(char) * size);
    if(id % global_corespernode == 0)
    {
        unlink(nfile_filename);
        rmdir(nfile_dirname);
        mkdir(nfile_dirname, 0777);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int64_t fd = open(nfile_filename, O_RDWR | O_CREAT, 0644);
    double t_start = 0.0;
    double t_stop = 0.0;
    double t_elapsed = 0.0;
    double sum_t_elapsed = 0.0;
    double t_min = 0.0;
    double t_max = 0.0;
    pthread_t thread;

    //lustre_lock(fd);

    int64_t * ns = (int64_t *)malloc(sizeof(int64_t));
    *ns = (global_corespernode - 1) * numrep;

    if(id % global_corespernode == 0)
    {
        global_sw = size;
        global_offset = 0;
        pthread_create(&thread, NULL, gaa_global_offset_thread, ns);
    }

    /* wait for everyone to open the file */
    MPI_Barrier(MPI_COMM_WORLD);

    t_start = MPI_Wtime();
    for(i = 0 ; i < numrep ; i++)
    {
        if(id % global_corespernode == 0)
        {
            offset = gaa_get_local_offset(size);
        }
        else
        {
            MPI_Status status;
            int64_t in_arg[2];
            int64_t out_arg[1];

            in_arg[0] = id;
            in_arg[1] = size;

            MPI_Send(&in_arg[0], 2, MPI_LONG_LONG_INT, group * global_corespernode, 0, MPI_COMM_WORLD);
            MPI_Recv(&out_arg[0], 1, MPI_LONG_LONG_INT, group * global_corespernode, 0, MPI_COMM_WORLD, &status);

            offset = out_arg[0];
        }
        if(enable_trace)
        {
            fprintf(stderr, "rank %i: i = %lli, offset = %lli, size = %lli\n", id, i,
                offset, size);
        }
        pwrite(fd, buffer, size, offset);
    }
    //lustre_unlock(fd);
    close(fd);
    MPI_Barrier(MPI_COMM_WORLD);
    if(id % global_corespernode == 0)
    {
        struct stat stat_buf;
        stat(nfile_filename, &stat_buf);
        if(stat_buf.st_size != (int64_t)(global_corespernode * numrep * size))
        {
            fprintf(stderr, "%s:%i ERROR: unexpected file size\n", __func__,
                    __LINE__);
            fprintf(stderr, "%s:%i actual: %lli, expected = %lli\n", __func__,
                    __LINE__, stat_buf.st_size, (int64_t)(global_corespernode * numrep *
                        size));
        }
        unlink(nfile_filename);
        rmdir(nfile_dirname);
    }
    MPI_Barrier(MPI_COMM_WORLD);
    t_stop = MPI_Wtime();
    t_elapsed = (t_stop - t_start);

    /* report results */
    MPI_Reduce(&t_elapsed, &sum_t_elapsed, 1, MPI_DOUBLE, MPI_SUM, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_max, 1, MPI_DOUBLE, MPI_MAX, 0,
            MPI_COMM_WORLD);
    MPI_Reduce(&t_elapsed, &t_min, 1, MPI_DOUBLE, MPI_MIN, 0,
            MPI_COMM_WORLD);

    if(id == 0)
    {
        fprintf(stderr, "file-per-ost results: %lf\n", t_max);
    }

    free(buffer);
    free(ns);
}

int parse_options(int argc, char ** argv)
{
    int mode_set = 0;
    int file_set = 0;
    int cpn_set = 0;
    int bs_set = 0;
    int nr_set = 0;

    while(1)
    {
        char c = 0;
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"trace", 0, 0, 't'},
            {"verbose", 0, 0, 'v'},
            {"file", 1, 0, 'f'},
            {"mode", 1, 0, 'm'},
            {"grouplocks", 0, 0, 'g'},
            {"cpn", 1, 0, 'c'},
            {"buffersize", 1, 0, 'b'},
            {"numreps", 1, 0, 'n'},
            {"help", 0, 0, 'h'},
            {0, 0, 0, 0}
        };

        c = getopt_long(argc, argv, "gtvf:m:c:b:n:h", long_options, &option_index);

        if(c == -1)
        {
            break;
        }

        switch(c)
        {
            case 't':
            {
                enable_trace = 1;
                break;
            }
            case 'g':
            {
                enable_grouplocks = 1;
                break;
            }
            case 'v':
            {
                enable_verbose = 1;
                break;
            }
            case 'f':
            {
                file_set = 1;
                memcpy(FILENAME, optarg, strlen(optarg) + 1);
                break;
            }
            case 'm':
            {
                mode_set = 1;
                enable_mode = atoi(optarg);
                break;
            }
            case 'c':
            {
                cpn_set = 1;
                global_corespernode = atoi(optarg);
                break;
            }
            case 'b':
            {
                bs_set = 1;
                global_buffersize = atoll(optarg);
                break;
            }
            case 'n':
            {
                nr_set = 1;
                global_numreps = atoi(optarg);
                break;
            }
            case 'h':
            {
                fprintf(stderr, "%s -f filename -m mode -c corespernode -b buffersize -n numreps [-htv]", argv[0]);
                fprintf(stderr, "\tsupported modes:\n");
                fprintf(stderr, "\t\t0 = 1 file, unique stripe per node\n");
                fprintf(stderr, "\t\t1 = 1 file, round robin stripe between nodes\n");
                fprintf(stderr, "\t\t2 = 1 file, global atomic append\n");
                fprintf(stderr, "\t\t3 = 1 file, stripe atomic append\n");
                fprintf(stderr, "\t\t4 = N file, file per OST\n");
                return -1;
            }
        };
    }
    if(mode_set && file_set && cpn_set && bs_set && nr_set)
        return 0;
    return -1;
}

int main(int argc, char ** args)
{
    int rank = 0;
    int size = 0;
    char hostname[HOSTNAMELEN];
    int p = 0;

    MPI_Init_thread(&argc, &args, MPI_THREAD_MULTIPLE, &p);

    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    gethostname(hostname, HOSTNAMELEN);
   
    parse_options(argc, args);

    if(enable_verbose)
    { 
        fprintf(stderr, "%i / %i on %s\n", rank, size, hostname);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    /* run test */
    switch(enable_mode)
    {
        case 0:
        {
            run_io_test_uniqost(global_buffersize, rank, size, global_numreps);
            break;
        }
        case 1:
        {
            run_io_test_rrost(global_buffersize, rank, size, global_numreps);
            break;
        }
        case 2:
        {
            run_io_test_aaost(global_buffersize, rank, size, global_numreps);
            break;
        }
        case 3:
        {
            run_io_test_gaaost(global_buffersize, rank, size, global_numreps);
            break;
        }
        case 4:
        {
            run_io_test_nfile(global_buffersize, rank, size, global_numreps);
            break;
        }
        default:
            break;
    };

    MPI_Finalize();

    return 0;
}
