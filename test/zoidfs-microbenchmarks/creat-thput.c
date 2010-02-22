/* Multi-client create throughput microbenchmark */

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

#include "mpi.h"
#include "thput.h"
#include "zoidfs/zoidfs.h"

#define OP1 0
#define OP2 1
#define MAXOP 2

static unsigned int estale_counts[MAXOP];

static int init_estale_counts(void)
{
    int i = 0;
    for(i = 0 ; i < MAXOP ; i++)
    {
        estale_counts[i] = 0;
    }

    return 0;
}

static int inc_estale_count(int id)
{
    estale_counts[id] += 1;

    return 0;
}

static void __attribute__((noreturn)) usage(char *progname) {
    fprintf(stderr, "Usage: %s <PVFS dir>\n", progname);
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
}

static void __attribute__((noreturn)) fatal_perror(char *progname, char *msg) {
    fprintf(stderr, "%s: %s", progname, msg);
    MPI_Abort(MPI_COMM_WORLD, 1);
    exit(1);
}

static inline int double_compare(const void *x1, const void *x2) {
    const double *d1 = x1;
    const double *d2 = x2;
    double diff = *d1 - *d2;

    return (diff < 0. ? -1 : (diff > 0. ? 1 : 0.));
}

static void print_stats(double *v, int numproc, char * cmd) {
    int i;
    double max = 0.;
    double avg = 0.;
    double stddev = 0.;
    double median = 0.;
    double min = 1.0e20;

    if (outer_iters > 0) {
        for (i = 0; i < outer_iters; i++) {
            v[i] = (double)(numproc*inner_iters)/v[i];  /* convert to thput */
            if (v[i] < min)
                min = v[i];
            if (v[i] > max)
                max = v[i];
            avg += (1.0 / v[i]); /* use harmonic mean... since this is an average of rates */
        }
        avg = (double) outer_iters / avg;

        if (outer_iters > 1) {
            for (i = 0; i < outer_iters; i++) {
                double diff = v[i] - avg;
                stddev += diff * diff;
            }
            stddev /= (double) (outer_iters - 1);
            stddev = sqrt(stddev);
        }

        if (1) {
            double *w;
            w = malloc(outer_iters * sizeof(*w));
            memcpy(w, v, outer_iters * sizeof(*v));
            qsort(w, outer_iters, sizeof(*w), double_compare);
            median = w[outer_iters/2];
            free(w);
        }
    }

    /* Print timing stats */
    printf("np %d %s thput avg %.3f +/- %.3f min %.3f max %.3f med %.3f\n",
            numproc, cmd, avg, stddev, min, max, median);
}

int main(int argc, char **argv) {
    double *v;
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_handle_t handle;
    double start = 0., end = 0.;
    zoidfs_handle_t basedir_handle;
    char filename[] = "creat-thput";
    char entry_name[ZOIDFS_NAME_MAX];
    int i, j, created = 0, numproc, rank, ret;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    init_estale_counts();

    if (argc < 2) {
        usage(argv[0]);
    }

    ret = zoidfs_init();
    if(ret != ZFS_OK) {
        fatal_perror(argv[0], "zoidfs_init() failed.\n");
    }

    ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle, ZOIDFS_NO_OP_HINT);
    if(ret != ZFS_OK) {
        fatal_perror(argv[0], "zoidfs_lookup() failed.\n");
    }

    /* Set the attributes */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0600;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* Do a noop to bring up the connections to the I/O node */
    zoidfs_null();

    /* alloc variables */
    v = (double *)malloc(outer_iters * sizeof(double));
    if (!v) {
        fatal_perror(argv[0], "malloc failed.\n");
    }

    /* run the test */
    for (i = 0; i < outer_iters; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) {
            start = MPI_Wtime();
        }

        for (j = 0; j < inner_iters; j++) {
            sprintf(entry_name, "%s-%d-%d.%d", filename, rank, i, j);
            do
            {
                int local_created = 0;
                ret = zoidfs_create(&basedir_handle, entry_name, NULL, &sattr,
                                    &handle, &local_created, ZOIDFS_NO_OP_HINT);

                /* if we created the file but could not complete beacuse of a stale, still count the file as created */
                if(local_created)
                {
                    created = local_created;
                }

                if(ret == ZFSERR_STALE)
                {
                    inc_estale_count(OP1);

                    /* lookup the base handle again since we got a stale... */
                    int _ret = 0;
                    memset(&basedir_handle, 0, sizeof(basedir_handle));
                    _ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle, ZOIDFS_NO_OP_HINT);
                    if(_ret != ZFS_OK) {
                        fatal_perror(argv[0], "ESTALE zoidfs_lookup() / basedir handle revlaidate failed.\n");
                    }
                }
            }while(ret == ZFSERR_STALE);
            if(ret != ZFS_OK) {
                fprintf(stderr, "ret code = %i\n", ret);
                fatal_perror(argv[0], "zoidfs_create() failed.\n");
            }
            assert(created);
        }

        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) {
            end = MPI_Wtime();
            v[i] = (end - start);
        }
    }

    /* Delete the files */
    for (i = 0; i < outer_iters; i++) {
        for (j = 0; j < inner_iters; j++) {
            sprintf(entry_name, "%s-%d-%d.%d", filename, rank, i, j);
            do
            {
                ret = zoidfs_remove(&basedir_handle, entry_name, NULL, NULL, ZOIDFS_NO_OP_HINT);
                if(ret == ZFSERR_STALE)
                {
                    inc_estale_count(OP2);

                    /* lookup the base handle again since we got a stale... */
                    int _ret = 0;
                    memset(&basedir_handle, 0, sizeof(basedir_handle));
                    _ret = zoidfs_lookup(NULL, NULL, argv[1], &basedir_handle, ZOIDFS_NO_OP_HINT);
                    if(_ret != ZFS_OK) {
                        fatal_perror(argv[0], "ESTALE zoidfs_lookup() / basedir handle revlaidate failed.\n");
                    }
                }
            }while(ret == ZFSERR_STALE);
            if(ret != ZFS_OK) {
                fatal_perror(argv[0], "zoidfs_remove() failed.\n");
            }
        }
    }

    /* Print stats */
    if (rank == 0) {
        print_stats(v, numproc, argv[0]);
    }

    free(v);
    zoidfs_finalize();
    MPI_Finalize();

    return 0;
}
