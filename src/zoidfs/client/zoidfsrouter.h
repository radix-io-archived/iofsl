#ifndef ZOIDFS_ROUTER_H
#define ZOIDFS_ROUTER_H

#include <bmi.h>

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

size_t zoidfs_router_get_server_start(int id);
size_t zoidfs_router_get_server_end(int id);

int zoidfs_router_find_start_server(size_t start);
int zoidfs_router_find_end_server(size_t end);
int zoidfs_router_get_next_server(int cur_server);

int zoidfs_router_get_num_servers();
size_t zoidfs_router_get_block_size();

/* MPI overloads */
int MPI_Init(int * args, char *** argv);
int MPI_Finalize();

#endif
