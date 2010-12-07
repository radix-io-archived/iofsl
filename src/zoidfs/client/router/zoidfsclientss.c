/*
 * This wrapper segments the procs in MPI_COMM_WORLD into IOFSL groups.
 *  Each IOFSL group communicates with a single IOFSL server.
 */
#include <mpi.h>

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <bmi.h>
#include <assert.h>

#include "zoidfs/client/router/zoidfsrouter.h"
#include "zoidfs/client/router/zoidfsclientbl.h"
#include "zoidfs/client/zoidfsclient.h"
#include "zoidfs/client/router/zoidfsclienthcache.h"
#include "zoidfs/client/bmi_comm.h"
#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

int zoidfs_router_bmi_addr_setup_static_server()
{
    int bret = BMI_addr_lookup(&(zoidfs_router_get_region_list()[0].bmi_addr), zoidfs_router_get_region_list()[0].iofsl_addr);
    if(bret < 0)
    {
        fprintf(stderr, "zoidfs_init: BMI_addr_lookup() failed, ion_name = %s.\n", zoidfs_router_get_region_list()[0].iofsl_addr);
        return -1;
    }
    zoidfs_router_set_num_servers(1);

    return 0;
}

/*
 * Static server overloads
 */
int MPI_Init_static_server(int * args, char *** argv)
{
    int rank = 0;
    int size = 0;
    int num_ioservers = atoi(getenv("ZOIDFS_NUM_IOSERVERS"));
    int io_server_rank = 0;
    char * client_config_fn = (char *)malloc(sizeof(char) * 1024);
    int ccfd = 0;

    /* init MPI */
    int ret = PMPI_Init(args, argv);

    /* set the num server variable */
    zoidfs_router_set_num_servers(num_ioservers);

    /* get the proc rank and size */
    PMPI_Comm_size(MPI_COMM_WORLD, &size);
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* compute the IO server rank this process will communicate with */
    io_server_rank = (int)((1.0 * rank / size) * num_ioservers);
    zoidfs_router_set_io_server_rank(io_server_rank);
    zoidfs_router_set_io_server_subrank(rank % (size / num_ioservers));

    zoidfs_router_server_region_t * l = (zoidfs_router_server_region_t *)malloc(sizeof(zoidfs_router_server_region_t));
    l[0].iofsl_addr = (char *)malloc(sizeof(char) * 1024);

    /* setup the io server addr to use */
    sprintf(client_config_fn, "./defaultclientconfig.cf.%i.%i.%i", size, num_ioservers, io_server_rank);

    /* get the address from the config file */
    ccfd = open(client_config_fn, O_RDONLY);
    if(ccfd < 0)
    {
        fprintf(stderr, "failed to open client config, file = %s\n", client_config_fn);
    }

    if(read(ccfd, l[0].iofsl_addr, 1024) > 0)
    {
        l[0].iofsl_addr[strlen(l[0].iofsl_addr) - 1] = '\0';
    }
    else
    {
        fprintf(stderr, "could not read the address\n");
    }

    /* close the fd */
    close(ccfd);

    /* cleanup */
    free(client_config_fn);

    zoidfs_router_set_region_list(l);

    return ret;
}

int MPI_Finalize_static_server()
{
    //return PMPI_Finalize();
    return 0;
}

/* overload of zoidfs_init */
int zoidfs_init_static_server(void)
{
    int ret = ZFS_OK;
    char * pipeline_size = NULL;

#ifdef ZFS_USE_XDR_SIZE_CACHE
    /* get the values for the size cache */
    zoidfs_xdr_size_processor_cache_init();
#endif

    assert(sizeof(size_t) == sizeof(unsigned long));

    /* Initialize BMI */
    ret = BMI_initialize(NULL, NULL, 0);
    if (ret < 0) {
        fprintf(stderr, "zoidfs_init: BMI_initialize() failed.\n");
        return ZFSERR_OTHER;
    }

    /* Create a new BMI context */
    ret = BMI_open_context(zoidfs_client_get_context());
    if (ret < 0) {
        fprintf(stderr, "zoidfs_init: BMI_open_context() failed.\n");
        return ZFSERR_OTHER;
    }

    /* setup the zoidfs router interface */
    zoidfs_router_bmi_addr_setup_static_server();

    /* set default peer addr */
    zoidfs_client_swap_addr(zoidfs_router_get_default_addr());
    zoidfs_client_set_def_addr(zoidfs_router_get_default_addr());

    /* setup the pipeline size */

    /* ask BMI for the max buffer size it can handle... */
    int psiz = 0;
    ret = BMI_get_info(*(zoidfs_router_get_default_addr()), BMI_CHECK_MAXSIZE, (void *)&psiz);
    if(ret)
    {
       fprintf(stderr, "zoidfs_init: BMI_get_info(BMI_CHECK_MAXSIZE) failed.\n");
       return ZFSERR_OTHER;
    }
    zoidfs_client_set_pipeline_size((size_t)psiz);

    /* check the for the PIPELINE_SIZE env variable */
    pipeline_size = getenv(PIPELINE_SIZE_ENV);
    if(pipeline_size)
    {
        int requested = atoi (pipeline_size);
        if (requested > psiz)
        {
           fprintf (stderr, "zoidfs_init: reducing pipelinesize to BMI max"
                 " (%i bytes)\n", psiz);
        }
        else
        {
            zoidfs_client_set_pipeline_size((size_t)requested);
        }
    }

#ifdef HAVE_BMI_ZOID_TIMEOUT
    {
        int timeout = 3600 * 1000;
        BMI_set_info(*(zoidfs_router_get_default_addr()), BMI_ZOID_POST_TIMEOUT, &timeout);
    }
#endif

    /* preallocate buffers */
#ifdef ZFS_BMI_FASTMEMALLOC
    zfs_bmi_client_sendbuf = BMI_memalloc(*(zoidfs_router_get_default_addr()), ZFS_BMI_CLIENT_SENDBUF_LEN, BMI_SEND);
    if(!zfs_bmi_client_sendbuf)
    {
        fprintf(stderr, "zoidfs_init: could not allocate send buffer for fast mem alloc.\n");
        return ZFSERR_OTHER;
    }

    zfs_bmi_client_recvbuf = BMI_memalloc(*(zoidfs_router_get_default_addr()), ZFS_BMI_CLIENT_RECVBUF_LEN, BMI_RECV);
    if(!zfs_bmi_client_recvbuf)
    {
        fprintf(stderr, "zoidfs_init: could not allocate recv buffer for fast mem alloc.\n");
        /* cleanup buffer */
        BMI_memfree (peer_addr, zfs_bmi_client_sendbuf,
              ZFS_BMI_CLIENT_SENDBUF_LEN, BMI_SEND);
        peer_addr = NULL;
        return ZFSERR_OTHER;
    }
#endif

    return ZFS_OK;
}

int zoidfs_finalize_static_server(void) {
    int ret = ZFS_OK;

    /* cleanup buffers */
#ifdef ZFS_BMI_FASTMEMALLOC
    if(zfs_bmi_client_sendbuf)
    {
        BMI_memfree (*(peer_addr), zfs_bmi_client_sendbuf,
              ZFS_BMI_CLIENT_SENDBUF_LEN, BMI_SEND);
        zfs_bmi_client_sendbuf = NULL;
    }

    if(zfs_bmi_client_recvbuf)
    {
        BMI_memfree (*(peer_addr), zfs_bmi_client_recvbuf,
              ZFS_BMI_CLIENT_RECVBUF_LEN, BMI_RECV);
        zfs_bmi_client_recvbuf = NULL;
    }
#endif

    BMI_close_context(zoidfs_client_get_context());

    /* Finalize BMI */
    ret = BMI_finalize();
    if (ret < 0) {
        fprintf(stderr, "zoidfs_finalize: BMI_finalize() failed.\n");
        exit(1);
    }
    return 0;
}

int MPI_File_open_static_server(MPI_Comm comm, char * filename, int amode, MPI_Info info, MPI_File * fh)
{
    int ret = 0;
    MPI_Comm dist_comm;
    int rank = 0;
    int size = 0;
    zoidfs_handle_t * local_handle = NULL;

    /* get the rank and size */
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
    PMPI_Comm_size(MPI_COMM_WORLD, &size);

    /* invoke the open */
    ret = PMPI_File_open(comm, filename, amode, info, fh);

    /* split the comm into groups based on IOFSL server assignement */
    PMPI_Comm_split(comm, zoidfs_router_get_io_server_rank(), zoidfs_router_get_io_server_subrank(), &dist_comm);

    /* create a handle */
    local_handle = (zoidfs_handle_t *)malloc(sizeof(zoidfs_handle_t));
 
    /* if the subrank is 0, generate a handle to the local IOFSL server */
    if(zoidfs_router_get_io_server_subrank() == 0)
    {
        /* perform the lookup */
        Pzoidfs_lookup(NULL, NULL, &(filename[7]), local_handle, NULL);
    }
   
    /* distribute the handle */ 
    PMPI_Bcast(local_handle, sizeof(zoidfs_handle_t), MPI_BYTE, 0, dist_comm);

    /* add the handle to the cache */
    zoidfs_client_handle_cache_add(fh, local_handle, strdup(&(filename[7])), 0);

    /* free the communicator */
    PMPI_Comm_free(&dist_comm);

    return 0;
}

int MPI_File_close_static_server(MPI_File * fh)
{
    int ret = 0;

    /* remove the handle from the cache */
    zoidfs_client_handle_list_cleanup(fh);

    /* invoke the close */
    ret = PMPI_File_close(fh);

    return ret;
}

int MPI_File_write_at_static_server(MPI_File fh, MPI_Offset offset, void * buf,
    int count, MPI_Datatype datatype, MPI_Status * status)
{
    int ret = 0;

    /* set the cur file handle */
    zoidfs_client_handle_cache_set_mpifh(&fh);

    /* get the file name */
    zoidfs_client_handle_cache_find_fn(zoidfs_client_handle_cache_get_mpifh());

    /* invoke the write op */
    ret = PMPI_File_write_at(fh, offset, buf, count, datatype, status);

    /* set the cur file handle to NULL */
    zoidfs_client_handle_cache_set_mpifhfn(NULL);
    zoidfs_client_handle_cache_set_mpifh(NULL);

    return ret;
}

int zoidfs_write_static_server(const zoidfs_handle_t *handle, size_t mem_count_,
                 const void *mem_starts[], const size_t mem_sizes_[],
                 size_t file_count_, const zoidfs_file_ofs_t file_starts[],
                 const zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint)
{
    return Pzoidfs_write(handle, mem_count_, mem_starts, mem_sizes_, file_count_, file_starts, file_sizes, op_hint);
}

zoidfs_router_t zoidfs_router_static_server = {
    .name = "ZOIDFS_STATIC_SERVER",
    .mpi_init = MPI_Init_static_server,
    .mpi_finalize = MPI_Finalize_static_server,
    .mpi_file_open = MPI_File_open_static_server,
    .mpi_file_close = MPI_File_close_static_server,
    .mpi_file_write_at = MPI_File_write_at_static_server,
    .zoidfs_write = zoidfs_write_static_server,
    .zoidfs_init = zoidfs_init_static_server,
    .zoidfs_finalize = zoidfs_finalize_static_server,
};
