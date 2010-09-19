/*
 * zoidfsclientrouter.c
 * ZoidFS client router lib. This code routes I/O data segments to
 *  IOFSL servers that may own a specific FS block region. The goal
 *  with this code is to reduce lock contention within the FS file servers
 *  by minimizing the block / file region lock revocation be ensuring that
 *  as few IOFSL servers are writing to a specific block as possible. Ideally,
 *  there should be one server that writes to a specific block.
 *
 * Jason Cope <copej@mcs.anl.gov>
 *
 */
#define _GNU_SOURCE
#include <search.h>
#include <assert.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"
#include "c-util/tools.h"
#include "zoidfs/client/zoidfsrouter.h"
#include "zoidfs/client/zoidfsclient.h"

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

#include "c-util/lookup8.h"
#include "c-util/lookup3.h"
#include "c-util/quicklist.h"

/*
 * filename and handle caches
 */

static MPI_File * cur_mpi_fh = NULL;
static char * cur_mpi_fh_fn = NULL;

static QLIST_HEAD(zoidfs_client_handle_list);

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

zoidfs_client_handle_cache_t * zoidfs_client_handle_cache_create(MPI_File * mpi_fh, zoidfs_handle_t * zoidfs_fh, char * filename, int server_id)
{
    zoidfs_client_handle_cache_t * r = (zoidfs_client_handle_cache_t *)malloc(sizeof(zoidfs_client_handle_cache_t));

    r->mpi_fh = mpi_fh;
    r->zoidfs_fh = zoidfs_fh;
    r->filename = filename;
    r->server_id = server_id;

    //fprintf(stderr, "%s zoidfs handle = %p, mpi fh = %p, filename = %s, sid = %i\n", __func__, r->zoidfs_fh, r->mpi_fh, r->filename, r->server_id);

    return r;
}

void zoidfs_client_handle_cache_destroy(void * r)
{
    if(((zoidfs_client_handle_cache_t *)r)->filename)
    {
        free(((zoidfs_client_handle_cache_t *)r)->filename);
    }
    if(((zoidfs_client_handle_cache_t *)r)->server_id == 0)
    {
        free(((zoidfs_client_handle_cache_t *)r)->zoidfs_fh);
    }
    free(r);
}

void zoidfs_client_handle_cache_add(MPI_File * mpi_fh, zoidfs_handle_t * zoidfs_fh, char * fn, int sid)
{
    zoidfs_client_handle_cache_t * e = zoidfs_client_handle_cache_create(mpi_fh, zoidfs_fh, fn, sid);

    qlist_add_tail(&(e->list), &zoidfs_client_handle_list);
}

void zoidfs_client_handle_cache_find_fn(MPI_File * mpi_fh)
{
    zoidfs_client_handle_cache_t * item = NULL;
    zoidfs_client_handle_cache_t * witem = NULL;

    /* find the mpi file handle in the list */
    qlist_for_each_entry_safe(item, witem, &zoidfs_client_handle_list, list)
    {
        /* if this is the fh, save it and break */
        if(memcmp(item->mpi_fh, mpi_fh, sizeof(MPI_File)) == 0)
        {
            cur_mpi_fh_fn = item->filename;
            //qlist_del(&(item->list));
            break;
        }
    }
}

zoidfs_client_handle_cache_t * zoidfs_client_handle_cache_find_by_fn_sid(char * filename, int server_id)
{
    zoidfs_client_handle_cache_t * item = NULL;
    zoidfs_client_handle_cache_t * fitem = NULL;
    zoidfs_client_handle_cache_t * witem = NULL;

    /* find the mpi file handle in the list */
    qlist_for_each_entry_safe(item, witem, &zoidfs_client_handle_list, list)
    {
        /* if this is the fh, save it and break */
        if(strcmp(item->filename, filename) == 0 && item->server_id == server_id)
        {
            fitem = item;
            break;
        }
    }

    return fitem;
}

void zoidfs_client_handle_list_cleanup(MPI_File * mpi_fh)
{
    zoidfs_client_handle_cache_t * item = NULL;
    zoidfs_client_handle_cache_t * witem = NULL;
    zoidfs_client_handle_cache_t * oitem = NULL;

    /* find the mpi file handle in the list */
    qlist_for_each_entry_safe(item, witem, &zoidfs_client_handle_list, list)
    {
        /* if this is the fh, save it and break */
        if(memcmp(item->mpi_fh, mpi_fh, sizeof(MPI_File)) == 0)
        {
            oitem = item;
            qlist_del(&(item->list));
            break;
        }
    }

    /* find all of the matches in the list and delete them */
    qlist_for_each_entry_safe(item, witem, &zoidfs_client_handle_list, list)
    {
        /* if the node is for the same file, delete it */
        if(strcmp(oitem->filename, item->filename) == 0)
        {
            qlist_del(&(item->list));

            zoidfs_client_handle_cache_destroy(item);
        }
    }
}

/*
 * fwd decl for the block offset type
 */
typedef struct zoidfs_client_router_block_offset fwd_zoidfs_client_router_block_offset_t;

/*
 * block id type
 */
typedef struct zoidfs_client_router_block_id
{
    int block_id;
    int count;
    void * block_offsets_tree;
    fwd_zoidfs_client_router_block_offset_t ** ofs_range_list;
    int cur_index;
    zoidfs_handle_t * handle;
    int handle_server_id;
    zoidfs_op_hint_t * op_hint;
} zoidfs_client_router_block_id_t;

/* 
 * create a block id
 */
static zoidfs_client_router_block_id_t * zoidfs_client_router_block_id_create(int sid, zoidfs_handle_t * handle, int handle_server_id, zoidfs_op_hint_t * op_hint)
{
    zoidfs_client_router_block_id_t * r = (zoidfs_client_router_block_id_t *)malloc(sizeof(zoidfs_client_router_block_id_t));

    r->block_id = sid;
    r->count = 0;
    r->block_offsets_tree = NULL;
    r->ofs_range_list = NULL;
    r->cur_index = 0;
    r->handle = handle;
    r->handle_server_id = handle_server_id;
    r->op_hint = op_hint;

    return r;
}

/*
 * destroy the block id
 */
static void zoidfs_client_router_block_id_destroy(void * r)
{
    free(r);
}

/*
 * block offset type
 */
typedef struct zoidfs_client_router_block_offset
{
    int block_id;
    int block_offset;
    zoidfs_client_router_block_id_t * parent;

    size_t mem_count;
    void * mem_starts;
    size_t mem_sizes;

    size_t file_count;
    zoidfs_file_ofs_t file_starts;
    zoidfs_file_size_t file_sizes;
} zoidfs_client_router_block_offset_t;

/*
 * create a block offset
 */
static zoidfs_client_router_block_offset_t * zoidfs_client_router_block_offset_create(int sid, int so, zoidfs_client_router_block_id_t * p,
    size_t mc, void * mstart, size_t msize, size_t fc, zoidfs_file_ofs_t fstart, zoidfs_file_size_t fsize)
{
    zoidfs_client_router_block_offset_t * r = (zoidfs_client_router_block_offset_t *)malloc(sizeof(zoidfs_client_router_block_offset_t));

    r->block_id = sid;
    r->block_offset = so;
    r->parent = p;
    r->mem_count = mc;
    r->mem_starts = mstart;
    r->mem_sizes = msize;
    r->file_count = fc;
    r->file_starts = fstart;
    r->file_sizes = fsize;

    //fprintf(stderr, "%s sid = %i fstart = %lu, fsize = %lu\n", __func__, r->block_id, r->file_starts, r->file_sizes);

    return r;
}

/* 
 * destroy a block offset type
 */
static void zoidfs_client_router_block_offset_destroy(void * r)
{
    free(r);
}

/*
 * compare two block ids
 */
static int zoidfs_client_router_block_id_cmp(const void * a, const void * b)
{
    if(((zoidfs_client_router_block_id_t *)a)->block_id < ((zoidfs_client_router_block_id_t *)b)->block_id)
    {
        return -1;
    }
    else if(((zoidfs_client_router_block_id_t *)a)->block_id > ((zoidfs_client_router_block_id_t *)b)->block_id)
    {
        return 1;
    }
    return 0;
}

/*
 * compare two block offsets
 */
static int zoidfs_client_router_block_offset_cmp(const void * a, const void * b)
{
    zoidfs_client_router_block_offset_t * a_cast = (zoidfs_client_router_block_offset_t *)a;
    zoidfs_client_router_block_offset_t * b_cast = (zoidfs_client_router_block_offset_t *)b;

    if(a_cast->block_offset < b_cast->block_offset)
    {
        return -1;
    }
    else if(a_cast->block_offset > b_cast->block_offset)
    {
        return 1;
    }
    return 0;
}

/*
 * find a block id
 */
static zoidfs_client_router_block_id_t * zoidfs_client_router_block_id_find(void ** tree, int sid)
{
    zoidfs_client_router_block_id_t ** ret = 0;
    zoidfs_client_router_block_id_t s;

    s.block_id = sid;

    ret = tfind((void *)&s, tree, zoidfs_client_router_block_id_cmp);

    if(ret == NULL)
    {
        return NULL;
    }
    
    return (*ret);
}

/*
 * add / create  a new file domain (block)
 */
static int zoidfs_client_router_file_domain_add(void ** tree, int sid, int so,
    size_t mc, void * mstart, size_t msize, size_t fc, zoidfs_file_ofs_t fstart, zoidfs_file_size_t fsize,
    zoidfs_handle_t * handle, int handle_server_id, zoidfs_op_hint_t * op_hint)
{
    zoidfs_client_router_block_id_t * sid_node = NULL;
    zoidfs_client_router_block_offset_t * so_node = NULL;

    /* search for the sid node */
    sid_node = zoidfs_client_router_block_id_find(tree, sid);

    /* else, create a new node block id node*/
    if(!sid_node) 
    {
        sid_node = zoidfs_client_router_block_id_create(sid, handle, handle_server_id, op_hint);
        tsearch(sid_node, tree, zoidfs_client_router_block_id_cmp);
    }

    /* create the file domain and associate it with the block id node */
    so_node = zoidfs_client_router_block_offset_create(sid, so, sid_node, mc, mstart, msize, fc, fstart, fsize);
    tsearch(so_node, &(sid_node->block_offsets_tree), zoidfs_client_router_block_offset_cmp);
    sid_node->count++;
    
    return 0;
}

/*
 * build a new file domain for a write op by walking the request tree
 */
static void zoidfs_client_router_file_domain_build(const void * nodep, const VISIT which, const int UNUSED(depth))
{
    zoidfs_client_router_block_id_t * p = (*((zoidfs_client_router_block_offset_t **) nodep))->parent;

    switch(which)
    {
        case preorder:
            break;
        case endorder:
            break;

        /* build the file domain in postorder... ascending offset order */
        case postorder:
        case leaf:
        {
            p->ofs_range_list[p->cur_index] = *((zoidfs_client_router_block_offset_t **)nodep);
            p->cur_index++;
            break;
        }
        default:
            break;
    };
}

static void zoidfs_client_router_file_domain_write(const void * nodep, const VISIT which, const int UNUSED(depth))
{
    zoidfs_client_router_block_id_t * cur_node = *(zoidfs_client_router_block_id_t **)nodep;

    switch(which)
    {
        case preorder:
            break;
        case endorder:
            break;

        /* build the file domain in postorder... ascending offset order */
        case postorder:
        case leaf:
        {
            int cur_sid = cur_node->block_id;

            int sub_mem_count = cur_node->count;
            void ** sub_mem_starts = malloc(sizeof(void *) * cur_node->count);
            size_t * sub_mem_sizes = malloc(sizeof(size_t) * cur_node->count);
            int sub_file_count = cur_node->count;
            size_t * sub_file_starts = malloc(sizeof(size_t) * cur_node->count);
            size_t * sub_file_sizes = malloc(sizeof(size_t) * cur_node->count);
            int j = 0;

            /* build the file domain */
            cur_node->ofs_range_list = (zoidfs_client_router_block_offset_t **)malloc(sizeof(zoidfs_client_router_block_offset_t *) * cur_node->count);
            twalk(cur_node->block_offsets_tree, zoidfs_client_router_file_domain_build);

            /* build the sub write data */
            fprintf(stderr, "%s node = %p sub io cur_sid = %i count = %i\n", __func__, nodep, cur_sid, sub_mem_count);
            for(j = 0 ; j < sub_mem_count ; j++)
            {
                sub_mem_starts[j] = cur_node->ofs_range_list[j]->mem_starts;
                sub_mem_sizes[j] = cur_node->ofs_range_list[j]->mem_sizes;
                sub_file_starts[j] = cur_node->ofs_range_list[j]->file_starts;
                sub_file_sizes[j] = cur_node->ofs_range_list[j]->file_sizes;

                fprintf(stderr, "%s, sub io m start[%i] = %p size[%i] = %lu, f start[%i] = %lu, size[%i] = %lu\n", __func__, j, sub_mem_starts[j], j, sub_mem_sizes[j], j, sub_file_starts[j], j, sub_file_sizes[j]);
            }

            /* invoke the write operation for this sub range of the client write op */
            int ret = 0;
            zoidfs_client_handle_cache_t * hret = zoidfs_client_handle_cache_find_by_fn_sid(cur_mpi_fh_fn, cur_sid);

            zoidfs_client_swap_addr(zoidfs_router_get_addr(cur_sid));

            ret = Pzoidfs_write(hret->zoidfs_fh, sub_mem_count, (const void **)sub_mem_starts, sub_mem_sizes, sub_file_count, sub_file_starts, sub_file_sizes, cur_node->op_hint);
            if(ret != ZFS_OK)
            {
                fprintf(stderr, "%s: zoidfs_write failure ret = %i\n", __func__, ret); 
            }
            zoidfs_client_def_addr();

            /* delete the range list */
            free(cur_node->ofs_range_list);

            /* cleanup local variables */
            free(sub_mem_starts);
            free(sub_mem_sizes);
            free(sub_file_starts);
            free(sub_file_sizes);

            /* destroy the offset tree in the cur node */
            tdestroy(cur_node->block_offsets_tree, zoidfs_client_router_block_offset_destroy);
            cur_node->block_offsets_tree = NULL;
    
            break;
        }
        default:
            break;
    };
}

/*
 * zoidfs_write
 * This function implements the zoidfs write call.
 *
 * TODO: big assumption here... the mem and file data is equivalent (same
 *  number of segments, segments are of similar size). This needs to be generalized.
 */
int zoidfs_write(const zoidfs_handle_t *handle, size_t mem_count_,
                 const void *mem_starts[], const size_t mem_sizes_[],
                 size_t file_count_, const zoidfs_file_ofs_t file_starts[],
                 const zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint) {
    int ret = 0;
    size_t i = 0;

    void * write_data_tree = NULL;
    size_t total_bytes = 0;
    size_t total_bytes_mem = 0;

    /* check if the mem_count_ and file_count_ fields match */
#if 0
    fprintf(stderr, "%s file_count = %i mem_count = %i\n", __func__, mem_count_, file_count_);
    for(i = 0 ; i < file_count_ ; i++)
    {
        fprintf(stderr, "%s file %i file start = %lu file size = %lu\n", __func__, i, file_starts[i], file_sizes[i]);
        fprintf(stderr, "%s mem %i mem start = %p mem size = %lu\n", __func__, i, mem_starts[i], mem_sizes_[i]);
    }
#endif

    assert(mem_count_ == file_count_);

    /* get the total size of file data */
    for(i = 0 ; i < file_count_ ; i++)
    {
        total_bytes += file_sizes[i];
        total_bytes_mem += mem_sizes_[i];
    }

    /* check that the total bytes for mem and file match */
    assert(total_bytes == total_bytes_mem);

    /* scan the list of files and determine which server they should be sent to */
    for(i = 0 ; i < file_count_ ; i++)
    {
        /* compute the start, end servers. get the number of wrap arounds */
        int ss = zoidfs_router_find_start_server(file_starts[i]);

        size_t cur_bytes = 0;

        int cur_server = ss;
        int num_servers = zoidfs_router_get_num_servers();
        size_t block_size = zoidfs_router_get_block_size();
        size_t cur_server_bytes = 0;
        size_t cur_server_start = 0;
        size_t cur_server_end = 0;

        void * mstart = (void *)mem_starts[0];
        size_t msize = mem_sizes_[0];
        size_t msizeremain = mem_sizes_[0];
        int mcur = 0;
        size_t fstart = file_starts[0]; 
        size_t fsize = file_sizes[0];
        size_t fsizeremain = file_sizes[0];
        int fcur = 0;

        /* while there is still data to process */
        while(cur_bytes != total_bytes)
        {
            /* get the start and end offsets for the cur server */
            cur_server_start = zoidfs_router_get_server_start(cur_server);
            cur_server_end = zoidfs_router_get_server_end(cur_server);

            /* compute the cur server bytes */
            cur_server_bytes = cur_server_end - cur_server_start + 1;
           
            /* get the size of the bytes to transfer */ 
            if(cur_server_bytes > msizeremain)
            {
                msize = msizeremain;
                msizeremain = 0;
                fsize = fsizeremain;
                fsizeremain = 0;
            }
            else
            {
                msize = cur_server_bytes;
                msizeremain -= cur_server_bytes;
                fsize = cur_server_bytes;
                fsizeremain -= cur_server_bytes;
            }

            /* update the cur_bytes */
            cur_bytes += msize;

            /* add this region to the local file domain tree */
            int so = fstart / (int)block_size / num_servers;
            zoidfs_client_router_file_domain_add(&write_data_tree, cur_server, so, 1, mstart, msize, 1, fstart, fsize, handle, zoidfs_router_get_default_server_id(), op_hint);
            
            /* update the file and mem starts */
            if(msizeremain > 0)
            {
                mstart += msize;
            }
            else
            {
                mstart = (void *)mem_starts[++mcur];
            }
            if(fsizeremain > 0)
            {
                fstart += fsize;
            }
            else
            {
                fstart = file_starts[++fcur];
            }

            /* get the next server */
            cur_server = zoidfs_router_get_next_server(cur_server);
        }
    }

    /* build the file domain */ 
    twalk(write_data_tree, zoidfs_client_router_file_domain_write);
    tdestroy(write_data_tree, zoidfs_client_router_block_id_destroy);

    return ret;
}

/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
int zoidfs_read(const zoidfs_handle_t *handle, size_t mem_count_,
                void *mem_starts[], const size_t mem_sizes_[],
                size_t file_count_, const zoidfs_file_ofs_t file_starts[],
                const zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    ret = Pzoidfs_read(handle, mem_count_, mem_starts, mem_sizes_, file_count_, file_starts, file_sizes, op_hint);

    return ret;
}

int MPI_File_open(MPI_Comm comm, char * filename, int amode, MPI_Info info, MPI_File * fh)
{
    int ret = 0;
    int i = 0;
    MPI_Comm leader_comm, dist_comm;
    int rank = 0;
    int size = 0;

    /* get the rank and size */
    PMPI_Comm_rank(MPI_COMM_WORLD, &rank);
    PMPI_Comm_size(MPI_COMM_WORLD, &size);

    /* invoke the open */
    ret = PMPI_File_open(comm, filename, amode, info, fh);

    /* setup the handle cache */ 
    int n = zoidfs_router_get_num_servers();
    int group_size = size / n;
    zoidfs_handle_t * all_handles = (zoidfs_handle_t *)malloc(sizeof(zoidfs_handle_t) * n);

    /* if this is a leader, lookup the handle with the local IOFSL server, gather and distribute the handles */
    if(rank == zoidfs_router_get_leader_rank())
    {
        zoidfs_handle_t * local_handle = (zoidfs_handle_t *)malloc(sizeof(zoidfs_handle_t));

        /* split the comm into leaders and others */
        MPI_Comm_split(MPI_COMM_WORLD, 1, rank / group_size, &leader_comm);

        /* perform the lookup */
        zoidfs_client_swap_addr(zoidfs_router_get_addr(zoidfs_router_get_leader_rank()));
        Pzoidfs_lookup(NULL, NULL, &(filename[7]), local_handle, NULL);
        zoidfs_client_def_addr();

        /* exchange the local handle with all other leaders */
        PMPI_Allgather(local_handle, sizeof(zoidfs_handle_t), MPI_BYTE, all_handles, sizeof(zoidfs_handle_t), MPI_BYTE, leader_comm);

        /* cleanup */
        free(local_handle);
        PMPI_Comm_free(&leader_comm);
    }
    else
    {
        int ofs = (rank / group_size) + 1;

        /* split the comm into leaders and others */
        MPI_Comm_split(MPI_COMM_WORLD, 0, rank - ofs,  &leader_comm);
        PMPI_Comm_free(&leader_comm);
    }

    /* dist group variables */
    if(group_size > 1)
    {
        /* split into sub groups based on the default server */
        MPI_Comm_split(MPI_COMM_WORLD, rank % n, rank / n, &dist_comm);

        /* distribute the handles to the group */
        PMPI_Bcast(all_handles, sizeof(zoidfs_handle_t) * n, MPI_BYTE, 0, dist_comm);

        /* cleanup */
        PMPI_Comm_free(&dist_comm);
    }

    /* setup the handle lookup */
    for(i = 0 ; i < n ; i++)
    {
        zoidfs_client_handle_cache_add(fh, &(all_handles[i]), strdup(&(filename[7])), i);
    }

    /* wait for all of the processes to complete the open */
    PMPI_Barrier(MPI_COMM_WORLD);

    return ret;
}

int MPI_File_close(MPI_File * fh)
{
    int ret = 0;

    /* remove the file name from the cache if success */
    zoidfs_client_handle_list_cleanup(fh);

    /* invoke the close */
    ret = PMPI_File_close(fh);

    return ret;
}

int MPI_File_write_at(MPI_File fh, MPI_Offset offset, void * buf,
    int count, MPI_Datatype datatype, MPI_Status * status)
{
    int ret = 0;

    /* set the cur file handle */
    cur_mpi_fh = &fh;

    /* get the file name */
    zoidfs_client_handle_cache_find_fn(cur_mpi_fh);

    /* invoke the write op */
    ret = PMPI_File_write_at(fh, offset, buf, count, datatype, status);

    /* set the cur file handle to NULL */
    cur_mpi_fh = NULL;
    cur_mpi_fh_fn = NULL;

    return ret;
}
