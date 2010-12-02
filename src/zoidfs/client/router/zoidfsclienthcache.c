/*
 * zoidfsclienthcache.c
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

#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <mpi.h>

#include "c-util/lookup8.h"
#include "c-util/lookup3.h"
#include "c-util/quicklist.h"

#include "zoidfs/client/router/zoidfsclienthcache.h"

/*
 * filename and handle caches
 */
static MPI_File * cur_mpi_fh = NULL;
static char * cur_mpi_fh_fn = NULL;
static QLIST_HEAD(zoidfs_client_handle_list);

void zoidfs_client_handle_cache_set_mpifh(MPI_File * mpi_fh)
{
    cur_mpi_fh = mpi_fh;
}

MPI_File *  zoidfs_client_handle_cache_get_mpifh()
{
    return cur_mpi_fh;
}

void zoidfs_client_handle_cache_set_mpifhfn(char * mpi_fh_fn)
{
    cur_mpi_fh_fn = mpi_fh_fn;
}

char * zoidfs_client_handle_cache_get_mpifhfn()
{
    return cur_mpi_fh_fn;
}

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
