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
#include <bmi.h>
#include <assert.h>

#include "zoidfs/client/zoidfsrouter.h"
#include "zoidfs/client/zoidfsclient.h"
#include "zoidfs/client/bmi_comm.h"
#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

static size_t zoidfs_router_block_size = 0;
static int zoidfs_router_num_servers = 0;
static zoidfs_router_server_region_t * zoidfs_router_region_list = NULL;
static zoidfs_routing_mode_t mode = UNKNOWN;
static int defid = 0;
static int local_leader_rank = 0;

int zoidfs_router_get_leader_rank()
{
    return local_leader_rank;
}

size_t zoidfs_router_get_block_size()
{
    return zoidfs_router_block_size;
}

char * zoidfs_router_get_server_addr(int server_id)
{
    if(server_id < zoidfs_router_num_servers)
    {
        return zoidfs_router_region_list[server_id].iofsl_addr;
    }
    return NULL;
}

int zoidfs_router_bmi_addr_setup()
{
    int i = 0;
   
    if(mode == STATIC_SERVER)
    {
        int bret = BMI_addr_lookup(&(zoidfs_router_region_list[0].bmi_addr), zoidfs_router_region_list[0].iofsl_addr);
        if (bret < 0)
        {
            fprintf(stderr, "zoidfs_init: BMI_addr_lookup() failed, ion_name = %s.\n", zoidfs_router_region_list[0].iofsl_addr);
            return -1;
        }

        zoidfs_router_num_servers = 1;
    }
    else if(mode == SIMPLE_BLOCK)
    { 
        for(i = 0 ; i < zoidfs_router_num_servers ; i++)
        {
          fprintf(stderr, "%s sid = %i, def = %i, addr = %s\n", __func__, i, defid, zoidfs_router_region_list[i].iofsl_addr);
          int bret = BMI_addr_lookup(&(zoidfs_router_region_list[i].bmi_addr), zoidfs_router_region_list[i].iofsl_addr);
          if (bret < 0)
            {
                fprintf(stderr, "zoidfs_init: BMI_addr_lookup() failed, ion_name = %s.\n", zoidfs_router_region_list[i].iofsl_addr);
                return -1;
            }
        }
    }

    return 0;
}

static void zoidfs_router_simple_block_layout_setup(int num_servers, size_t block_size)
{
    int i = 0;
    size_t curofs = 0;

    /* setup static dim variables */
    zoidfs_router_block_size = block_size;
    zoidfs_router_num_servers = num_servers;

    zoidfs_router_region_list = (zoidfs_router_server_region_t *)malloc(sizeof(zoidfs_router_server_region_t) * num_servers);

    for(i = 0 ; i < num_servers ; i++)
    {
        zoidfs_router_region_list[i].id = i; 
        zoidfs_router_region_list[i].start = curofs;
        zoidfs_router_region_list[i].end = curofs + block_size - 1;
        curofs += block_size;
    }
}

static void zoidfs_router_simple_block_layout_cleanup()
{
    int i = 0;

    /* cleanup each server entry */
    for(i = 0 ; i < zoidfs_router_num_servers ; i++)
    {
        free(zoidfs_router_region_list[i].iofsl_addr);
        zoidfs_router_region_list[i].iofsl_addr = NULL;
    }

    /* reset all of the region variables */
    zoidfs_router_block_size = 0;
    zoidfs_router_num_servers = 0;
    
    free(zoidfs_router_region_list);
    zoidfs_router_region_list = NULL;
}

int zoidfs_router_get_num_servers()
{
    return zoidfs_router_num_servers;
}

int zoidfs_router_get_next_server(int cur_server)
{
    if(cur_server + 1 < zoidfs_router_num_servers)
    {
        return cur_server + 1;
    }
    return 0;
}

size_t zoidfs_router_get_server_start(int id)
{
    if(id >= 0 && id < zoidfs_router_num_servers)
    {
        return zoidfs_router_region_list[id].start;
    }
    return 0;
}

size_t zoidfs_router_get_server_end(int id)
{
    if(id >= 0 && id < zoidfs_router_num_servers)
    {
        return zoidfs_router_region_list[id].end;
    }
    return 0;
}

BMI_addr_t * zoidfs_router_get_default_addr()
{
    return &(zoidfs_router_region_list[defid].bmi_addr);
}

int zoidfs_router_get_default_server_id()
{
    return defid;
}

BMI_addr_t * zoidfs_router_get_addr(int id)
{
    if(id >= 0 && id < zoidfs_router_num_servers)
    {
        return &(zoidfs_router_region_list[id].bmi_addr); 
    }
    return NULL;
}

int zoidfs_router_find_start_server(size_t start)
{
    int start_server_ofs = 0;

    start_server_ofs = start / zoidfs_router_block_size;
    
    if(start_server_ofs < zoidfs_router_num_servers)
    {
        return start_server_ofs;
    }
    else
    {
        return start_server_ofs % zoidfs_router_num_servers;
    }

    return -1;
}

int zoidfs_router_find_end_server(size_t end)
{
    int end_server_ofs = 0;

    end_server_ofs = end / zoidfs_router_block_size;
    
    if(end_server_ofs < zoidfs_router_num_servers)
    {
        return end_server_ofs;
    }
    else
    {
        return end_server_ofs % zoidfs_router_num_servers;
    }

    return -1;
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

    /* get the proc rank and size */
    PMPI_Comm_size(MPI_COMM_WORLD, &size);
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* compute the IO server rank this process will communicate with */
    io_server_rank = (int)((1.0 * rank / size) * num_ioservers);

    zoidfs_router_region_list = (zoidfs_router_server_region_t *)malloc(sizeof(zoidfs_router_server_region_t));
    zoidfs_router_region_list[0].iofsl_addr = (char *)malloc(sizeof(char) * 1024);

    /* setup the io server addr to use */
    sprintf(client_config_fn, "./defaultclientconfig.cf.%i.%i.%i", size, num_ioservers, io_server_rank);

    /* get the address from the config file */
    ccfd = open(client_config_fn, O_RDONLY);
    if(ccfd < 0)
    {
        fprintf(stderr, "failed to open client config, file = %s\n", client_config_fn);
    }

    if(read(ccfd, zoidfs_router_region_list[0].iofsl_addr, 1024) > 0)
    {
        zoidfs_router_region_list[0].iofsl_addr[strlen(zoidfs_router_region_list[0].iofsl_addr) - 1] = '\0';
    }
    else
    {
        fprintf(stderr, "could not read the address\n");
    }

    /* close the fd */
    close(ccfd);

    /* cleanup */
    free(client_config_fn);

    return ret;
}

int MPI_Finalize_static_server()
{
    return PMPI_Finalize();
}

/*
 * Simple block layout overloads
 */
int MPI_Init_simple_block_layout_server(int * args, char *** argv)
{
    int rank = 0;
    int size = 0;
    int i = 0;
    int ccfd = 0;
    int num_ioservers = atoi(getenv("ZOIDFS_NUM_IOSERVERS"));
    size_t block_size = atoi(getenv("ZOIDFS_BLOCK_SIZE"));

    /* init MPI */
    int ret = PMPI_Init(args, argv);

    /* get the proc rank and size */
    PMPI_Comm_size(MPI_COMM_WORLD, &size);
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);

    /* compute the layout */
    zoidfs_router_simple_block_layout_setup(num_ioservers, block_size);

    /* rank 0 will get the server addresses and distribute them to the other clients */
    if(rank == 0)
    {
        /* create a string to hold the config file path */
        char * client_config_fn = (char *)malloc(sizeof(char) * 1024);

        /* for each iofsl server */
        for(i = 0 ; i < num_ioservers ; i++)
        {
            /* setup the io server addr to use */
            sprintf(&client_config_fn[0], "./defaultclientconfig.cf.%i.%i.%i", size, num_ioservers, i);

            /* setup the file name for the server / region */ 
            zoidfs_router_region_list[i].iofsl_addr = (char *)malloc(sizeof(char) * 128);
 
            /* get the address from the config file */
            ccfd = open(client_config_fn, O_RDONLY);
            if(ccfd < 0)
            {
                fprintf(stderr, "%s: failed to open client config, file = %s\n", __func__, client_config_fn);
            }

            /* store the address... will lookup it up later */
            if(read(ccfd, zoidfs_router_region_list[i].iofsl_addr, 128) <= 0)
            {
                fprintf(stderr, "%s: could not read the address\n", __func__);
            }

            /* remove the newline */
            zoidfs_router_region_list[i].iofsl_addr[strlen(zoidfs_router_region_list[i].iofsl_addr) - 1] = '\0';

            /* close the fd */
            close(ccfd);

            /* send this addr out to all of the other clients */
            PMPI_Bcast(zoidfs_router_region_list[i].iofsl_addr, 128, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    
        /* cleanup */
        free(client_config_fn);
    }
    /* all other ranks wait for the server addrs */
    else
    {
        defid = rank % num_ioservers; 
        for(i = 0 ; i < num_ioservers ; i++)
        {
            /* setup the address path */
            zoidfs_router_region_list[i].iofsl_addr = (char *)malloc(sizeof(char) * 128);

            /* get the address */
            PMPI_Bcast(zoidfs_router_region_list[i].iofsl_addr, 128, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    }

    /* assign the leaders */
    local_leader_rank = rank % num_ioservers;

    return ret;
}

int MPI_Finalize_simple_block_layout_server()
{
    int ret = PMPI_Finalize();

    zoidfs_router_simple_block_layout_cleanup();

    return ret;
}

/*
 * MPI overloads for zoidfs routing
 */
int MPI_Init(int * args, char *** argv)
{
    char * zoidfs_mode = getenv(ZOIDFS_ROUTING_MODE);

    /* static server mode */
    if(strcmp(zoidfs_mode, ZOIDFS_STATIC_SERVER) == 0)
    {
        mode = STATIC_SERVER;
        return MPI_Init_static_server(args, argv);
    }
    /* simple block mode */
    else if(strcmp(zoidfs_mode, ZOIDFS_SIMPLE_BLOCK_LAYOUT_SERVER) == 0)
    {
        mode = SIMPLE_BLOCK;
        return MPI_Init_simple_block_layout_server(args, argv);
    }

    /* could not find the mode */
    return -1;
}

int MPI_Finalize()
{
    /* static server mode */
    if(mode == STATIC_SERVER)
    {
        return MPI_Finalize_static_server();
    }
    /* simple block mode */
    else if(mode == SIMPLE_BLOCK)
    {
        return MPI_Finalize_simple_block_layout_server();
    }

    /* could not find the mode */
    return -1;
}

/* overload of zoidfs_init */
int zoidfs_init(void)
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
    zoidfs_router_bmi_addr_setup();

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
        return ZFSERR_OTHER;
    }
#endif

    return ZFS_OK;
}
