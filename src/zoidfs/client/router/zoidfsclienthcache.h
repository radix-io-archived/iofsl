#ifndef ZOIDFS_CLIENT_ROUTER_ZOIDFSCLIENTHCACHE_H
#define ZOIDFS_CLIENT_ROUTER_ZOIDFSCLIENTHCACHE_H

#include <mpi.h>

#include "c-util/quicklist.h"
#include <stdint.h>
#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

typedef struct zoidfs_client_handle_cache
{
    struct qlist_head list;
    MPI_File * mpi_fh;
    zoidfs_handle_t * zoidfs_fh;
    char * filename;
    int server_id;
    uint64_t sidfn_key;
    uint64_t fh_key;

} zoidfs_client_handle_cache_t;

void zoidfs_client_handle_cache_destroy(void * r);

zoidfs_client_handle_cache_t * zoidfs_client_handle_cache_create(MPI_File * mpi_fh, zoidfs_handle_t * zoidfs_fh, char * filename, int server_id);
void zoidfs_client_handle_cache_add(MPI_File * mpi_fh, zoidfs_handle_t * zoidfs_fh, char * fn, int sid);
void zoidfs_client_handle_cache_find_fn(MPI_File * mpi_fh);
void zoidfs_client_handle_list_cleanup(MPI_File * mpi_fh);
void zoidfs_client_handle_cache_set_mpifh(MPI_File * mpi_fh);
MPI_File *  zoidfs_client_handle_cache_get_mpifh();

zoidfs_client_handle_cache_t * zoidfs_client_handle_cache_find_by_fn_sid(char * filename, int server_id);
void zoidfs_client_handle_cache_set_mpifhfn(char * mpi_fh_fn);
char * zoidfs_client_handle_cache_get_mpifhfn();

#endif
