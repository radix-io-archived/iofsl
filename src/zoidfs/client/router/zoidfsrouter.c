/*
 * This library provides an interface for extending and building
 *  zoidfs clients with additional cababilities.
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
#include "zoidfs/client/router/zoidfsclientss.h"
#include "zoidfs/client/zoidfsclient.h"
#include "zoidfs/client/router/zoidfsclienthcache.h"
#include "zoidfs/client/bmi_comm.h"
#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

static size_t zoidfs_router_block_size = 0;
static int zoidfs_router_num_servers = 0;
static int zoidfs_router_io_server_rank = 0;
static int zoidfs_router_io_server_subrank = 0;
static zoidfs_router_server_region_t * zoidfs_router_region_list = NULL;
static int mode = -1;
static int defid = 0;
static int local_leader_rank = 0;

void zoidfs_router_set_io_server_rank(int r)
{
    zoidfs_router_io_server_rank = r;
}

void zoidfs_router_set_io_server_subrank(int r)
{
    zoidfs_router_io_server_subrank = r;
}

int zoidfs_router_get_io_server_rank()
{
    return zoidfs_router_io_server_rank;
}

int zoidfs_router_get_io_server_subrank()
{
    return zoidfs_router_io_server_subrank;
}

void zoidfs_router_set_defid(int id)
{
    defid = id;
}

void zoidfs_router_set_num_servers(int n)
{
    zoidfs_router_num_servers = n;
}

void zoidfs_router_set_block_size(size_t b)
{
    zoidfs_router_block_size = b;
}

zoidfs_router_server_region_t * zoidfs_router_get_region_list()
{
    return zoidfs_router_region_list;
}

void zoidfs_router_set_region_list(zoidfs_router_server_region_t * l)
{
    zoidfs_router_region_list = l;
}

void zoidfs_router_set_leader_rank(int r)
{
    local_leader_rank = r;
}

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

static zoidfs_router_t * zoidfs_routers[] = {
    &zoidfs_router_static_server,
    &zoidfs_router_simple_block_layout_server,
};

#define ZOIDFS_ROUTER_COUNT ((unsigned int) (sizeof(zoidfs_routers) / sizeof(zoidfs_routers[0])))

/*
 * MPI overloads for zoidfs routing
 */
int MPI_Init(int * args, char *** argv)
{
    unsigned int i = 0;
    char * zoidfs_mode = getenv(ZOIDFS_ROUTING_MODE);

    for( i = 0 ; i < ZOIDFS_ROUTER_COUNT ; i++)
    { 
        if(strcmp(zoidfs_mode, zoidfs_routers[i]->name) == 0)
        {
            mode = i;
            return zoidfs_routers[mode]->mpi_init(args, argv);
        }
    }
   
    /* could not find the router mode */ 
    mode = -1;

    /* could not find the mode */
    return -1;
}

int MPI_Finalize()
{
    int ret = 0;

    ret = zoidfs_routers[mode]->mpi_finalize();
    mode = -1;

    /* could not find the mode */
    return ret;
}

/* overload of zoidfs_init */
int zoidfs_init(void)
{
    return zoidfs_routers[mode]->zoidfs_init();
}

/* overload of zoidfs_init */
int zoidfs_finalize(void)
{
    return zoidfs_routers[mode]->zoidfs_finalize();
}

int MPI_File_open(MPI_Comm comm, MPICONST char * filename, int amode, MPI_Info
      info, MPI_File * fh)
{
    return zoidfs_routers[mode]->mpi_file_open(comm, filename, amode, info,
          fh);
}

int MPI_File_close(MPI_File * fh)
{
    return zoidfs_routers[mode]->mpi_file_close(fh);
}

int MPI_File_writer(MPI_File fh, MPI_Offset offset, MPICONST void * buf,
    int count, MPI_Datatype datatype, MPI_Status * status)
{
    return zoidfs_routers[mode]->mpi_file_write_at(fh, offset, buf, count,
          datatype, status);
}
