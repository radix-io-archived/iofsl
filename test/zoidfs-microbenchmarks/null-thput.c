/* Multi-client null throughput microbenchmark */

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
            avg += (1.0 / v[i]);
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
    double start = 0., end = 0.;
    int i, j, numproc, rank, ret;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 2) {
        usage(argv[0]);
    }

    ret = zoidfs_init();
    if(ret != ZFS_OK) {
        fatal_perror(argv[0], "zoidfs_init() failed.\n");
    }

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
            ret = zoidfs_null();
            if(ret != ZFS_OK) {
                fatal_perror(argv[0], "zoidfs_null() failed.\n");
            }
        }

        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == 0) {
            end = MPI_Wtime();
            v[i] = (end - start);
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
