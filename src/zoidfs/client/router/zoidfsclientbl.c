/*
 * zoidfsclientbl.c
 * ZoidFS client block layout lib. This code routes I/O data segments to
 *  IOFSL servers that may own a specific FS block region.
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
#include "zoidfs/client/router/zoidfsrouter.h"
#include "zoidfs/client/zoidfsclient.h"
#include "zoidfs/client/bmi_comm.h"

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <mpi.h>

#include "c-util/lookup8.h"
#include "c-util/lookup3.h"
#include "c-util/quicklist.h"

#include "zoidfs/client/router/zoidfsclienthcache.h"

int zoidfs_router_bmi_addr_setup_block_layout()
{
    int i = 0;
    for(i = 0 ; i < zoidfs_router_get_num_servers() ; i++)
    {
        //fprintf(stderr, "%s sid = %i, def = %i, addr = %s\n", __func__, i, defid, zoidfs_router_region_list[i].iofsl_addr);
        int bret = BMI_addr_lookup(&(zoidfs_router_get_region_list()[i].bmi_addr), zoidfs_router_get_region_list()[i].iofsl_addr);
        if(bret < 0)
        {
            fprintf(stderr, "zoidfs_init: BMI_addr_lookup() failed, ion_name = %s.\n", zoidfs_router_get_region_list()[i].iofsl_addr);
            return -1;
        }
    }
    return 0;
}

static void zoidfs_router_simple_block_layout_setup(int num_servers, size_t block_size)
{
    int i = 0;
    size_t curofs = 0;
    zoidfs_router_server_region_t * l = NULL;

    /* setup static dim variables */
    zoidfs_router_set_block_size(block_size);
    zoidfs_router_set_num_servers(num_servers);

    l = (zoidfs_router_server_region_t *)malloc(sizeof(zoidfs_router_server_region_t) * num_servers);

    for(i = 0 ; i < num_servers ; i++)
    {
        l[i].id = i;
        l[i].start = curofs;
        l[i].end = curofs + block_size - 1;
        curofs += block_size;
    }
    zoidfs_router_set_region_list(l);
}

static void zoidfs_router_simple_block_layout_cleanup()
{
    int i = 0;

    /* cleanup each server entry */
    for(i = 0 ; i < zoidfs_router_get_num_servers() ; i++)
    {
        free(zoidfs_router_get_region_list()[i].iofsl_addr);
        zoidfs_router_get_region_list()[i].iofsl_addr = NULL;
    }

    /* reset all of the region variables */
    zoidfs_router_set_block_size(0);
    zoidfs_router_set_num_servers(0);

    free(zoidfs_router_get_region_list());
    zoidfs_router_set_region_list(NULL);
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
            zoidfs_router_get_region_list()[i].iofsl_addr = (char *)malloc(sizeof(char) * 128);

            /* get the address from the config file */
            ccfd = open(client_config_fn, O_RDONLY);
            if(ccfd < 0)
            {
                fprintf(stderr, "%s: failed to open client config, file = %s\n", __func__, client_config_fn);
            }

            /* store the address... will lookup it up later */
            if(read(ccfd, zoidfs_router_get_region_list()[i].iofsl_addr, 128) <= 0)
            {
                fprintf(stderr, "%s: could not read the address\n", __func__);
            }

            /* remove the newline */
            zoidfs_router_get_region_list()[i].iofsl_addr[strlen(zoidfs_router_get_region_list()[i].iofsl_addr) - 1] = '\0';

            /* close the fd */
            close(ccfd);

            /* send this addr out to all of the other clients */
            PMPI_Bcast(zoidfs_router_get_region_list()[i].iofsl_addr, 128, MPI_CHAR, 0, MPI_COMM_WORLD);
        }

        /* cleanup */
        free(client_config_fn);
    }
    /* all other ranks wait for the server addrs */
    else
    {
        zoidfs_router_set_defid(rank % num_ioservers);
        for(i = 0 ; i < num_ioservers ; i++)
        {
            /* setup the address path */
            zoidfs_router_get_region_list()[i].iofsl_addr = (char *)malloc(sizeof(char) * 128);

            /* get the address */
            PMPI_Bcast(zoidfs_router_get_region_list()[i].iofsl_addr, 128, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
    }

    /* assign the leaders */
    zoidfs_router_set_leader_rank(rank % num_ioservers);

    return ret;
}

int MPI_Finalize_simple_block_layout_server()
{
    int ret = PMPI_Finalize();

    zoidfs_router_simple_block_layout_cleanup();

    return ret;
}

/* overload of zoidfs_init */
int zoidfs_init_simple_block_layout_server(void)
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
    zoidfs_router_bmi_addr_setup_block_layout();

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

int zoidfs_finalize_simple_block_layout_server(void) {
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

    BMI_close_context(*(zoidfs_client_get_context()));

    /* Finalize BMI */
    ret = BMI_finalize();
    if (ret < 0) {
        fprintf(stderr, "zoidfs_finalize: BMI_finalize() failed.\n");
        exit(1);
    }
    return 0;
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
static zoidfs_client_router_block_id_t * zoidfs_client_router_block_id_create(int sid, const zoidfs_handle_t * handle, int handle_server_id, zoidfs_op_hint_t * op_hint)
{
    zoidfs_client_router_block_id_t * r = (zoidfs_client_router_block_id_t *)malloc(sizeof(zoidfs_client_router_block_id_t));

    r->block_id = sid;
    r->count = 0;
    r->block_offsets_tree = NULL;
    r->ofs_range_list = NULL;
    r->cur_index = 0;
    r->handle = (zoidfs_handle_t *)handle;
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
    const zoidfs_handle_t * handle, int handle_server_id, zoidfs_op_hint_t * op_hint)
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
            zoidfs_client_handle_cache_t * hret = zoidfs_client_handle_cache_find_by_fn_sid(zoidfs_client_handle_cache_get_mpifhfn(), cur_sid);

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
int zoidfs_write_simple_block_layout(const zoidfs_handle_t *handle, size_t mem_count_,
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

int MPI_File_open_simple_block_layout_server(MPI_Comm comm, char * filename, int amode, MPI_Info info, MPI_File * fh)
{
    fprintf(stderr, "%s:%i enter\n", __func__, __LINE__);
    PMPI_Barrier(MPI_COMM_WORLD);
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
        PMPI_Comm_split(MPI_COMM_WORLD, 1, rank / group_size, &leader_comm);

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
        PMPI_Comm_split(MPI_COMM_WORLD, 0, rank - ofs,  &leader_comm);
        PMPI_Comm_free(&leader_comm);
    }

    /* dist group variables */
    if(group_size > 1)
    {
        /* split into sub groups based on the default server */
        PMPI_Comm_split(MPI_COMM_WORLD, rank % n, rank / n, &dist_comm);

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
    fprintf(stderr, "%s:%i exit, fh = %p\n", __func__, __LINE__, fh);
    PMPI_Barrier(MPI_COMM_WORLD);

    return ret;
}

int MPI_File_close_simple_block_layout_server(MPI_File * fh)
{
    fprintf(stderr, "%s:%i enter\n", __func__, __LINE__);
    int ret = 0;

    /* remove the file name from the cache if success */
    zoidfs_client_handle_list_cleanup(fh);

    /* invoke the close */
    ret = PMPI_File_close(fh);

    fprintf(stderr, "%s:%i exit\n", __func__, __LINE__);
    return ret;
}

int MPI_File_write_at_simple_block_layout_server(MPI_File fh, MPI_Offset offset, void * buf,
    int count, MPI_Datatype datatype, MPI_Status * status)
{
    fprintf(stderr, "%s:%i enter\n", __func__, __LINE__);
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

    fprintf(stderr, "%s:%i exit\n", __func__, __LINE__);
    return ret;
}

zoidfs_router_t zoidfs_router_simple_block_layout_server = {
    .name = "ZOIDFS_SIMPLE_BLOCK_LAYOUT_SERVER",
    .mpi_init = MPI_Init_simple_block_layout_server,
    .mpi_finalize = MPI_Finalize_simple_block_layout_server,
    .mpi_file_open = MPI_File_open_simple_block_layout_server,
    .mpi_file_close = MPI_File_close_simple_block_layout_server,
    .mpi_file_write_at = MPI_File_write_at_simple_block_layout_server,
    .zoidfs_write = zoidfs_write_simple_block_layout,
    .zoidfs_init = zoidfs_init_simple_block_layout_server,
    .zoidfs_finalize = zoidfs_finalize_simple_block_layout_server,
};
