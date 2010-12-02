#ifndef ZOIDFS_CLIENT_ROUTER_ZOIDFSROUTER_H
#define ZOIDFS_CLIENT_ROUTER_ZOIDFSROUTER_H

#include <bmi.h>
#include <mpi.h>
#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

/* server modes */
#define ZOIDFS_ROUTING_MODE "ZOIDFS_ROUTING_MODE"
#define ZOIDFS_SIMPLE_BLOCK_LAYOUT_SERVER "ZOIDFS_SIMPLE_BLOCK_LAYOUT_SERVER"
#define ZOIDFS_STATIC_SERVER "ZOIDFS_STATIC_SERVER"

typedef enum
{
    UNKNOWN=-1,
    STATIC_SERVER=0,
    SIMPLE_BLOCK,
    ROUTING_MODE_MAX
} zoidfs_routing_mode_t;

typedef struct zoidfs_router_server_region
{
    int id;
    size_t start;
    size_t end;
    char * iofsl_addr;
    BMI_addr_t bmi_addr;
} zoidfs_router_server_region_t;

/* router helper functions */
char * zoidfs_router_get_server_addr(int server_id);
BMI_addr_t * zoidfs_router_get_addr(int id);
BMI_addr_t * zoidfs_router_get_default_addr();
int zoidfs_router_get_default_server_id();
int zoidfs_router_get_leader_rank();
int zoidfs_router_bmi_addr_setup();
zoidfs_router_server_region_t * zoidfs_router_get_region_list();
void zoidfs_router_set_region_list(zoidfs_router_server_region_t * l);

size_t zoidfs_router_get_server_start(int id);
size_t zoidfs_router_get_server_end(int id);

int zoidfs_router_find_start_server(size_t start);
int zoidfs_router_find_end_server(size_t end);
int zoidfs_router_get_next_server(int cur_server);

int zoidfs_router_get_num_servers();
size_t zoidfs_router_get_block_size();
void zoidfs_router_set_block_size(size_t b);
void zoidfs_router_set_num_servers(int n);
void zoidfs_router_set_defid(int id);
void zoidfs_router_set_leader_rank(int r);

void zoidfs_router_set_io_server_rank(int r);
void zoidfs_router_set_io_server_subrank(int r);
int zoidfs_router_get_io_server_rank();
int zoidfs_router_get_io_server_subrank();

/* MPI overloads */
int MPI_Init(int * args, char *** argv);
int MPI_Finalize();
int MPI_File_open(MPI_Comm comm, char * filename, int amode, MPI_Info info, MPI_File * fh);
int MPI_File_close(MPI_File * fh);
int MPI_File_write_at(MPI_File fh, MPI_Offset offset, void * buf,
    int count, MPI_Datatype datatype, MPI_Status * status);

/* typedefs of required methods for zoidfs router implementations */
typedef int (* zoidfs_router_mpi_init_t)(int * args, char *** argv);
typedef int (* zoidfs_router_mpi_finalize_t)();
typedef int (* zoidfs_router_mpi_file_open_t)(MPI_Comm comm, char * filename, int amode, MPI_Info info, MPI_File * fh);
typedef int (* zoidfs_router_mpi_file_close_t)(MPI_File * fh);
typedef int (* zoidfs_router_mpi_file_write_at_t)(MPI_File fh, MPI_Offset offset, void * buf,
    int count, MPI_Datatype datatype, MPI_Status * status);
typedef int (* zoidfs_router_zoidfs_write_t)(const zoidfs_handle_t * handle, size_t mem_count,
    const void * mem_starts[], const size_t mem_sizes[], size_t file_count,
    const zoidfs_file_ofs_t file_starts[], const zoidfs_file_size_t file_sizes[],
    zoidfs_op_hint_t * op_hint);
typedef int (* zoidfs_router_zoidfs_init_t)();
typedef int (* zoidfs_router_zoidfs_finalize_t)();
 
typedef struct zoidfs_router
{
    char * name;
    zoidfs_router_mpi_init_t mpi_init;
    zoidfs_router_mpi_finalize_t mpi_finalize;
    zoidfs_router_mpi_file_open_t mpi_file_open;
    zoidfs_router_mpi_file_close_t mpi_file_close;
    zoidfs_router_mpi_file_write_at_t mpi_file_write_at;
    zoidfs_router_zoidfs_init_t zoidfs_init;
    zoidfs_router_zoidfs_finalize_t zoidfs_finalize;
    zoidfs_router_zoidfs_write_t zoidfs_write;
} zoidfs_router_t;
#endif
