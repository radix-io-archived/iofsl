/*
 * This wrapper segments the procs in MPI_COMM_WORLD into IOFSL groups.
 *  Each IOFSL group communicates with a single IOFSL server. So,
 *  apps using this wrapper can communicate with different IOFSL servers
 *  and we support a crude / hacky form of multi server I/O forwarding. Each
 *  client is bound to a specific IOFSL server and cannot comunicate with
 *  other servers.
 */
#include <mpi.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int MPI_Init(int * args, char *** argv)
{
    int rank = 0;
    int size = 0;
    int i = 0;
    int shared_hosts = 0;
    char * hostname;
    int num_ioservers = atoi(getenv("ZOIDFS_NUM_IOSERVERS"));
    int io_server_rank = 0;
    char * client_config_fn = (char *)malloc(sizeof(char) * 1024);
    char * iofsl_addr = (char * )malloc(sizeof(char) * 1024);
    int ccfd = 0;

    /* init MPI */
    int ret = PMPI_Init(args, argv);

    /* get the proc rank and size */
    PMPI_Comm_size(MPI_COMM_WORLD, &size);
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* compute the IO server rank this process will communicate with */
    io_server_rank = (int)floor((1.0 * rank / size) * num_ioservers);

    /* setup the io server addr to use */
    sprintf(client_config_fn, "./defaultclientconfig.cf.%i.%i.%i", size, num_ioservers, io_server_rank);

    /* get the address from the config file */
    ccfd = open(client_config_fn, O_RDONLY);
    if(ccfd < 0)
    {
        fprintf(stderr, "failed to open client config\n");
    }

    /* overwrite the ZOIDFS_ION_NAME */
    if(read(ccfd, iofsl_addr, 1024) > 0)
    {
        setenv("ZOIDFS_ION_NAME", iofsl_addr, 1);
    }
    else
    {
        fprintf(stderr, "could not read the address\n");
        setenv("ZOIDFS_ION_NAME", "", 1);
    }

    /* close the fd */
    close(ccfd);

    /* cleanup */
    free(iofsl_addr);
    free(client_config_fn);

    return ret;
}
