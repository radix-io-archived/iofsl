#define _GNU_SOURCE
#include <search.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "zoidfs/zoidfs.h"
#include "zoidfs/dispatcher/zint-handler.h"
#include "c-util/tools.h"
#include "c-util/configfile.h"
#include "c-util/sha1.h"

#include "globus_ftp_client.h"

#include "c-util/lookup8.h"

/* driver configuration variables */
static int connection_caching_enabled = 0;
static int stripes_enabled = 0;
static int tcp_buffer_size = 0;
static int parallel_streams = 0; 

static globus_mutex_t lookup_ctl_lock;

/* zoidfs gridftp op */
typedef struct zoidfs_gridftp_op
{
    /* callback locks */
    globus_mutex_t lock;
    globus_cond_t cond;

    /* handle flags */
    globus_bool_t file_exists;
    globus_bool_t exists_done; 
    globus_bool_t file_created;
    globus_bool_t create_done; 
    globus_bool_t create_wb_done;
    globus_bool_t file_removed;
    globus_bool_t remove_done;
    globus_bool_t dir_created;
    globus_bool_t mkdir_done;
    globus_bool_t file_resized;
    globus_bool_t resize_done;
    globus_bool_t resize_wb_done;
    globus_bool_t wrote_file;
    globus_bool_t write_ctl_done;
    globus_bool_t read_file;
    globus_bool_t read_ctl_done;
    globus_bool_t renamed_file;
    globus_bool_t rename_done;

    /* data amounts */
    globus_size_t bytes_written;
    globus_size_t bytes_read;

} zoidfs_gridftp_op_t;

/* zoidfs gridftp handle */
typedef struct zoidfs_gridftp_handle
{
    /* key */
    uint64_t key;

    /* file path */
    char * file_path;

    /* zoidfs handle */
    zoidfs_handle_t zfs_handle;

    /* gridftp client handles and attrs */
    globus_ftp_client_handle_t gftpfh;
    globus_ftp_client_handleattr_t hattr;
    globus_ftp_client_operationattr_t oattr;

    /* callback locks */
    globus_mutex_t lock;
    globus_cond_t cond;

    /* handle flags */
    globus_bool_t file_exists;
    globus_bool_t exists_done; 
    globus_bool_t file_created;
    globus_bool_t create_done; 
    globus_bool_t create_wb_done;
    globus_bool_t file_removed;
    globus_bool_t remove_done;
    globus_bool_t dir_created;
    globus_bool_t mkdir_done;
    globus_bool_t file_resized;
    globus_bool_t resize_done;
    globus_bool_t resize_wb_done;
    globus_bool_t wrote_file;
    globus_bool_t write_ctl_done;
    globus_bool_t read_file;
    globus_bool_t read_ctl_done;
    globus_bool_t renamed_file;
    globus_bool_t rename_done;
    globus_bool_t getattr_done;

    /* */
    globus_size_t bytes_written;
    globus_size_t bytes_read;

    int validHandle;

} zoidfs_gridftp_handle_t;

/* handle create and destroy funcs */
static zoidfs_gridftp_handle_t * zoidfs_gridftp_handle_create()
{
    zoidfs_gridftp_handle_t * h = NULL;

    /* allocate a new handle */
    h = (zoidfs_gridftp_handle_t *)malloc(sizeof(zoidfs_gridftp_handle_t));

    h->key = 0;
    h->file_path = NULL;
    h->bytes_written = 0;
    h->bytes_read = 0;
    h->validHandle = 0;

    /* init the locks */
    globus_mutex_init(&(h->lock), GLOBUS_NULL);
    globus_cond_init(&(h->cond), GLOBUS_NULL);

    /* init handle flags to false */    
    h->file_exists = GLOBUS_FALSE;
    h->exists_done = GLOBUS_FALSE;
    h->file_created = GLOBUS_FALSE;
    h->create_done = GLOBUS_FALSE;
    h->create_wb_done = GLOBUS_FALSE;
    h->file_removed = GLOBUS_FALSE;
    h->remove_done = GLOBUS_FALSE;
    h->dir_created = GLOBUS_FALSE;
    h->mkdir_done = GLOBUS_FALSE;
    h->file_resized = GLOBUS_FALSE;
    h->resize_done = GLOBUS_FALSE;
    h->resize_wb_done = GLOBUS_FALSE;
    h->wrote_file = GLOBUS_FALSE;
    h->write_ctl_done = GLOBUS_FALSE;
    h->read_file = GLOBUS_FALSE;
    h->read_ctl_done = GLOBUS_FALSE;
    h->renamed_file = GLOBUS_FALSE;
    h->rename_done = GLOBUS_FALSE;
    h->getattr_done = GLOBUS_FALSE;

    return h;
}

static void zoidfs_gridftp_handle_destroy(void * h)
{
    globus_bool_t ret;

    /* deallocate the handle */
    if(h)
    {
        zoidfs_gridftp_handle_t * hh = (zoidfs_gridftp_handle_t *)h;

        /* free the file path made by strdup */
        free(hh->file_path);

        /* cleanup handle info */
        ret = globus_ftp_client_handle_destroy(&(hh->gftpfh));
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not destroy handle\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
        }

        /* cleanup handle attr info */
        ret = globus_ftp_client_handleattr_destroy(&(hh->hattr));
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not destroy handle attr\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
        }

        /* cleanup op attr info */
        ret = globus_ftp_client_operationattr_destroy(&(hh->oattr));
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not destroy operation attr\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
        }

        /* free the handle */
        free(hh);
    }
}

/* handle tree */
static void * zoidfs_gridftp_handle_tree = NULL;

/* handle tree functions */
static int zoidfs_gridftp_handle_compare(const void * a, const void * b)
{
    if(((zoidfs_gridftp_handle_t *)a)->key < ((zoidfs_gridftp_handle_t *)b)->key)
    {
        return -1;
    }
    else if(((zoidfs_gridftp_handle_t *)a)->key > ((zoidfs_gridftp_handle_t *)b)->key)
    {
        return 1;
    }

    /* the keys are equal */
    return 0;
}

static int zoidfs_gridftp_handle_tree_add(zoidfs_gridftp_handle_t * h)
{
    void * ret = NULL;

    /* add the key to the tree */
    ret = tsearch((void *)h, &zoidfs_gridftp_handle_tree, zoidfs_gridftp_handle_compare);

    return 0;
}

static zoidfs_gridftp_handle_t * zoidfs_gridftp_handle_tree_find(uint64_t hv)
{
    zoidfs_gridftp_handle_t sitem;
    sitem.key = hv;
    zoidfs_gridftp_handle_t ** ret = NULL;

    ret = tfind((void *)&sitem, &zoidfs_gridftp_handle_tree, zoidfs_gridftp_handle_compare);

    if(ret == NULL)
    {
        return NULL;
    }

    return (*ret);
}

static void * zoidfs_gridftp_handle_tree_remove(uint64_t hv)
{
    zoidfs_gridftp_handle_t sitem;
    sitem.key = hv;
    zoidfs_gridftp_handle_t ** ret = NULL;

    ret = tdelete((void *)&sitem, &zoidfs_gridftp_handle_tree, zoidfs_gridftp_handle_compare);

    if(ret == NULL)
    {
        return NULL;
    }

    return *(ret);
}

static int zoidfs_gridftp_handle_tree_cleanup()
{
    tdestroy(zoidfs_gridftp_handle_tree, zoidfs_gridftp_handle_destroy);

    return 0;
}

static void exists_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->file_exists = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    zgh->exists_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void remove_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->file_removed = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    zgh->remove_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void mkdir_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->dir_created = GLOBUS_TRUE;
    }

    /* update done flag and signal */
    zgh->mkdir_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void rename_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->renamed_file = GLOBUS_TRUE;
    }

    /* update done flag and signal */
    zgh->rename_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void create_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->file_created = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    zgh->create_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void create_data_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error,
    globus_byte_t * UNUSED(buffer), globus_size_t UNUSED(length), globus_off_t UNUSED(offset), globus_bool_t UNUSED(eof))
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->file_created = GLOBUS_TRUE;
    }
  
    /* update done flag and signal */ 
    zgh->create_wb_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void getattr_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    
    /* update done flag and signal */
    zgh->getattr_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void resize_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->file_resized = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    zgh->resize_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void resize_data_cb(void * myargs, globus_ftp_client_handle_t * handle, globus_object_t * error,
    globus_byte_t * buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
            globus_ftp_client_register_write(handle, buffer, length, offset, eof, resize_data_cb, myargs);
    }
  
    /* update done flag and signal */ 
    zgh->resize_wb_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void write_ctl_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->wrote_file = GLOBUS_TRUE;
    }
    zgh->write_ctl_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void write_data_cb(void * myargs, globus_ftp_client_handle_t * handle, globus_object_t * error,
                                   globus_byte_t * buffer, globus_size_t length, globus_off_t offset,
                                   globus_bool_t eof)
{
    globus_size_t * bytes_written = NULL;
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;

    bytes_written = &(zgh->bytes_written);
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }

    *bytes_written += length;
    if(!eof)
    {
        globus_ftp_client_register_write(handle, buffer, length, offset, GLOBUS_TRUE, write_data_cb, (void *)bytes_written);
    }
}

static void read_ctl_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        zgh->read_file = GLOBUS_TRUE;
    }
    zgh->read_ctl_done = GLOBUS_TRUE;
    globus_cond_signal(&(zgh->cond));
}

static void read_data_cb(void * myargs, globus_ftp_client_handle_t * handle, globus_object_t * error,
                                   globus_byte_t * buffer, globus_size_t length, globus_off_t UNUSED(offset),
                                   globus_bool_t eof)
{
    globus_size_t * bytes_read = NULL;
    zoidfs_gridftp_handle_t * zgh = (zoidfs_gridftp_handle_t *)myargs;

    bytes_read = &(zgh->bytes_read);
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }

    *bytes_read += length;
    if(!eof)
    {
        globus_ftp_client_register_read(handle, buffer, length, read_data_cb, (void *)bytes_read);
    }
}

int zoidfs_gridftp_filename_to_handle(const char * buf, zoidfs_handle_t ** h)
{
    /* compute the hash of the filename */
    uint64_t hv = hash((void *)buf, strlen(buf) + 1, 0);

    /* get the handle from the tree and store it in handle arg */
    *h = &(zoidfs_gridftp_handle_tree_find(hv))->zfs_handle;
    
    return ZFS_OK;
}

static int zoidfs_gridftp_handle_to_filename (const zoidfs_handle_t * handle,
      char * buf, int bufsize, zoidfs_gridftp_handle_t ** zgh)
{
    /* get the gridftp handle hash value from the zoidfs handle */
    uint64_t hv = *((uint64_t *)(handle->data + 4));

    /* get the gridftp handle from the tree */
    *zgh = zoidfs_gridftp_handle_tree_find(hv);

    /* check that the buffer can hold the max path length */
    assert(bufsize >= ZOIDFS_PATH_MAX);

    /* cp the path from the gridftp handle */
    strncpy(buf, (*zgh)->file_path, zfsmin((unsigned int)bufsize, strlen((*zgh)->file_path)));

    return ZFS_OK;
}

static int zoidfs_gridftp_zfs_handle_create(const uint64_t hv, zoidfs_handle_t * h)
{
    uint64_t * hi = (uint64_t *)(h->data + 4);

    *hi = hv;

    return ZFS_OK;
}

static uint64_t zoidfs_gridftp_zfs_handle_get(const zoidfs_handle_t * h)
{
    uint64_t * hi = (uint64_t *)(h->data + 4);

    return *hi;
}

static int zoidfs_gridftp_convert_filename(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            char * gftp_path)
{
    if(full_path)
    {
        /*
         * Big assumption here!
         * GridFTP path has the following form
         *  /transfer_type/host/port/path/to/file
         * transfer type is ftp or gsiftp
         */
        unsigned int i = 0;
        unsigned int j = 0;
        char * str = NULL;
        char * saveptr = NULL;
        char gftp_path_cp[1024];
        char * gftp_method = NULL;
        char * gftp_host = NULL;
        char * gftp_port = NULL;
        char * gftp_file_path = NULL;

        /* make a copy of the path */
        strcpy(gftp_path_cp, full_path);

        /* split the string into tokens */
        for(i = 1 , str = (char *)gftp_path_cp ; ; i++, str = NULL)
        {
            char * token = strtok_r(str, "/", &saveptr);
            if(token == NULL)
            {
                break;
            }

            /* method */
            if(i == 1)
            {
                gftp_method = strdup(token);
            }
            /* host */
            else if(i == 2)
            {
                gftp_host = strdup(token);
            }
            /* port */
            else if(i == 3)
            {
                gftp_port = strdup(token);
            }
        }

        /* find the start of the file path */
        i = 0;
        j = 0;
        while( j < 4 && i < strlen(full_path))
        {
            if(full_path[i] == '/')
            {
                j++;
            }
            i++;
        }
        gftp_file_path = (char *)&(full_path[i - 1]);

        /* right now, use ftp */
        sprintf(gftp_path, "%s://%s:%s%s", gftp_method, gftp_host, gftp_port, gftp_file_path);
        if(gftp_method)
        {
            free(gftp_method);
            gftp_method = NULL;
        }
        if(gftp_host)
        {
            free(gftp_host);
            gftp_host = NULL;
        }
        if(gftp_port)
        {
            free(gftp_port);
            gftp_port = NULL;
        }
    }
    else if(component_name != NULL && parent_handle != NULL)
    {
        zoidfs_gridftp_handle_t * pzgh = NULL;
        uint64_t hv = zoidfs_gridftp_zfs_handle_get(parent_handle);

        pzgh = zoidfs_gridftp_handle_tree_find(hv);

        /* if we found the parent handle */
        if(pzgh)
        {
            sprintf(gftp_path, "%s%s", pzgh->file_path, component_name);
        }
    }

    return ZFS_OK;
}

static zoidfs_gridftp_handle_t * zoidfs_gridftp_setup_gridftp_handle(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * handle)
{
    globus_result_t ret;
    char gftp_path[1024];
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hv = 0;
    char * gftp_method = NULL;
    char * gftp_host = NULL;
    char * gftp_port = NULL;
    char * gftp_file_path = NULL;

    /* we have the full path, break it into tokens */
    if(full_path)
    {
        /*
         * Big assumption here!
         * GridFTP path has the following form
         *  /transfer_type/host/port/path/to/file
         * transfer type is ftp or gsiftp
         */
        unsigned int i = 0;
        unsigned int j = 0;
        char * str = NULL;
        char * saveptr = NULL;
        char gftp_path_cp[1024];

        /* make a copy of the path */
        strcpy(gftp_path_cp, full_path);

        /* split the string into tokens */
        for(i = 1 , str = (char *)gftp_path_cp ; ; i++, str = NULL)
        {
            char * token = strtok_r(str, "/", &saveptr);
            if(token == NULL)
            {
                break;
            }

            /* method */
            if(i == 1)
            {
                gftp_method = strdup(token);
            }
            /* host */
            else if(i == 2)
            {
                gftp_host = strdup(token);
            }
            /* port */
            else if(i == 3)
            {
                gftp_port = strdup(token);
            }
        }
       
        /* find the start of the file path */ 
        i = 0;
        j = 0;
        while( j < 4 && i < strlen(full_path))
        {
            if(full_path[i] == '/')
            {
                j++;
            }
            i++;
        }
        gftp_file_path = (char *)&(full_path[i - 1]);

        /* right now, use ftp */
        sprintf(gftp_path, "%s://%s:%s%s", gftp_method, gftp_host, gftp_port, gftp_file_path);
        if(gftp_method)
        {
            free(gftp_method);
            gftp_method = NULL;
        }
        if(gftp_host)
        {
            free(gftp_host);
            gftp_host = NULL;
        }
        if(gftp_port)
        {
            free(gftp_port);
            gftp_port = NULL;
        }

        /* hash and search for the handle */
        hv = hash((void *)gftp_path, strlen(gftp_path) + 1, 0);
        zgh = zoidfs_gridftp_handle_tree_find(hv);
    }
    else if(component_name != NULL && parent_handle != NULL) 
    {
        zoidfs_gridftp_handle_t * pzgh = NULL;

        hv = zoidfs_gridftp_zfs_handle_get(parent_handle);
        pzgh = zoidfs_gridftp_handle_tree_find(hv);

        /* if we found the parent handle */
        if(pzgh)
        {
            sprintf(gftp_path, "%s%s", pzgh->file_path, component_name);
            hv = hash((void *)gftp_path, strlen(gftp_path) + 1, 0);
            zgh = zoidfs_gridftp_handle_tree_find(hv);
        }
    }

    /* handle was found, return that handle */
    if(zgh != NULL)
    {
        fprintf(stderr, "%s : found the handle\n", __func__);
        /* make a copy of the handle if a valid handle pointer was provided */
        if(handle != NULL)
        {
            fprintf(stderr, "%s : copy the handle\n", __func__);
            memcpy(handle, &(zgh->zfs_handle), sizeof(zoidfs_handle_t));
        }
        return zgh;
    }
    else
    {
        fprintf(stderr, "%s : did not find the handle\n", __func__);
    }

    zgh = zoidfs_gridftp_handle_create();

    zgh->file_path = strdup(gftp_path);
    zgh->key = hv;

    /* only init a handle if one was passed to this func */
    if(handle != NULL)
    {
        zoidfs_gridftp_zfs_handle_create(hv, handle);
        memcpy(&(zgh->zfs_handle), handle, sizeof(zoidfs_handle_t));
    }

    /* init the handle attr */
    ret = globus_ftp_client_handleattr_init(&(zgh->hattr));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not init handle\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return NULL;
    }

    /* init the operation */
    ret = globus_ftp_client_operationattr_init(&(zgh->oattr));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not init op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return NULL;
    }

    /* enable gridftp connection caching */
    if(connection_caching_enabled)
    {
        ret = globus_ftp_client_handleattr_set_cache_all(&(zgh->hattr), GLOBUS_TRUE);
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not cache connection\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
            return NULL;
        }
    }

    /* set the authorization params */
    ret = globus_ftp_client_operationattr_set_authorization(&(zgh->oattr), GSS_C_NO_CREDENTIAL, "copej", "", "copej", "");
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not set authorization\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return NULL;
    }

    /* handle init */
    ret = globus_ftp_client_handle_init(&(zgh->gftpfh), &(zgh->hattr));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR reinint handle, %s\n", __func__, globus_object_printable_to_string(globus_error_get(ret)));
        fprintf(stderr, "%s : exit\n", __func__);
        return NULL;
    }

    /* add the handle to the handle tree */
    zoidfs_gridftp_handle_tree_add(zgh);

    return zgh;
}

/* just return... GridFTP does not have a null op */
static int zoidfs_gridftp_null(void)
{
    return ZFS_OK;
}

static int zoidfs_gridftp_getattr(const zoidfs_handle_t * handle,
                       zoidfs_attr_t * attr,
                       zoidfs_op_hint_t * UNUSED(hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hkey = 0;
    globus_off_t fsize = 0;
    globus_abstime_t mtime;
    globus_size_t stat_buffer_size = 4096;
    globus_byte_t * stat_buffer;

    stat_buffer = (globus_byte_t *)malloc(stat_buffer_size * sizeof(globus_byte_t));

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);

    if(zgh == NULL)
    {
        fprintf(stderr, "%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

#if 0
    /* setup gridftp create */
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->getattr_done = GLOBUS_FALSE;

    /* get the file size */
    fprintf(stderr, "%s : file name = %s\n", __func__, zgh->file_path);
    ret = globus_ftp_client_size(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &(fsize), getattr_cb, zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register client size\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->getattr_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    attr->size = (uint64_t)fsize;
    globus_mutex_unlock(&(zgh->lock));
#endif

    /* get the file size */
    zgh->getattr_done = GLOBUS_FALSE;
    ret = globus_ftp_client_modification_time(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &mtime, getattr_cb, zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register client size\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->getattr_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));

    /* update all of the times to the mtime */
    attr->mtime.seconds = mtime.tv_sec; 
    attr->mtime.nseconds = 0; 
    //attr->mtime.nseconds = mtime.tv_usec * 1000; 
    attr->atime.seconds = mtime.tv_sec; 
    attr->atime.nseconds = 0; 
    //attr->atime.nseconds = mtime.tv_usec * 1000; 
    attr->ctime.seconds = mtime.tv_sec; 
    //attr->ctime.nseconds = mtime.tv_usec * 1000;
    attr->ctime.nseconds = 0;
 
    globus_mutex_unlock(&(zgh->lock));

    /* get the file size */
    zgh->getattr_done = GLOBUS_FALSE;
    ret = globus_ftp_client_stat(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &stat_buffer, &stat_buffer_size, getattr_cb, zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register client size\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->getattr_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    fprintf(stderr, "%s : stat buffer = %s\n", __func__, stat_buffer);
    globus_mutex_unlock(&(zgh->lock));

    return ZFS_OK;
}

/* TODO */
static int zoidfs_gridftp_setattr(const zoidfs_handle_t * UNUSED(handle),
                       const zoidfs_sattr_t * UNUSED(sattr),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_lookup(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_handle_t * handle,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;

    //globus_mutex_lock(&lookup_ctl_lock);
    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, handle);

    /* if the handle is already valid, just return */
    if(zgh->validHandle)
    {
        //globus_mutex_unlock(&lookup_ctl_lock);
        return ZFS_OK;
    }

    /* setup gridftp exist test */
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->file_exists = GLOBUS_FALSE;
    zgh->exists_done = GLOBUS_FALSE;

    /* run exist test */
    fprintf(stderr, "%s : register lookup on %s, key = %lu\n", __func__, zgh->file_path, zgh->key);
    ret = globus_ftp_client_exists(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), exists_cb, (void *)zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not run gftp exist op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->exists_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    globus_mutex_unlock(&(zgh->lock));

    if(zgh->file_exists != GLOBUS_TRUE)
    {
        zgh->validHandle = 1;
        //globus_mutex_unlock(&lookup_ctl_lock);
        return ZFSERR_EXIST;
    }
    //globus_mutex_unlock(&lookup_ctl_lock);
   
    return ZFS_OK;
}

/* not supported by GridFTP??? */
static int zoidfs_gridftp_readlink(const zoidfs_handle_t * UNUSED(handle),
                        char * UNUSED(buffer),
                        size_t UNUSED(buffer_length),
                        zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zoidfs_gridftp_read(const zoidfs_handle_t * handle,
                    size_t mem_count,
                    void * mem_starts[],
                    const size_t mem_sizes[],
                    size_t file_count,
                    const uint64_t file_starts[],
                    uint64_t file_sizes[],
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hkey = 0;
    globus_off_t start = 0;
    globus_off_t end = 0;
    size_t i = 0;
    size_t start_i = 0;
    size_t end_i = 0;
    char * gftp_buffer = NULL;

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);

    if(zgh == NULL)
    {
        fprintf(stderr, "%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* setup gridftp create */
    //globus_mutex_lock(&lookup_ctl_lock);
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->read_file = GLOBUS_FALSE;
    zgh->read_ctl_done = GLOBUS_FALSE;

    /* find the start and end offsets for this operation */
    start = file_starts[0];
    start_i = 0;
    end = file_starts[0] + file_sizes[0];
    end_i = 0;
    for( i = 1 ; i < file_count ; i++)
    {
        /* if the current index start is less than the least known start */
        if(file_starts[i] < (uint64_t)start)
        {
            start = file_starts[i];
            start_i = i;
        }

        /* if the current index end is greater than the greatest known end */
        if(file_starts[i] + file_sizes[i] > (uint64_t)end)
        {
            end = file_starts[i] + file_sizes[i];
            end_i = 0;
        }
    }
    fprintf(stderr, "%s : start = %lu, end = %lu\n", __func__, start, end); 

    /* get the file size */
    ret = globus_ftp_client_partial_get(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, start, end, read_ctl_cb, zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register partial get\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }
    fprintf(stderr, "%s : get registered\n", __func__); 

    /* allocate the read buffer */
    gftp_buffer = (char *) malloc(sizeof(char) * (end - start));

    /* register the read */
    ret = globus_ftp_client_register_read(&(zgh->gftpfh), (globus_byte_t *)gftp_buffer, (globus_size_t)(end - start), read_data_cb, (void *) zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register read\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->read_ctl_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    globus_mutex_unlock(&(zgh->lock));
    //globus_mutex_unlock(&lookup_ctl_lock);

    /* copy the buffer segments into the user args */
    for(i = 0 ; i < mem_count ; i++)
    {
        const uint64_t foff = file_starts[i] - start;
        memcpy(mem_starts[i], &(gftp_buffer[foff]), mem_sizes[i]);
    }   

    /* free the read buffer */ 
    free(gftp_buffer);

    return ZFS_OK;
}

static int zoidfs_gridftp_write(const zoidfs_handle_t * handle,
                     size_t mem_count,
                     const void * mem_starts[],
                     const size_t mem_sizes[],
                     size_t file_count,
                     const uint64_t file_starts[],
                     uint64_t file_sizes[],
                     zoidfs_op_hint_t * UNUSED(op_hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hkey = 0;
    globus_off_t start = 0;
    globus_off_t end = 0;
    size_t i = 0;
    size_t start_i = 0;
    size_t end_i = 0;

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);

    if(zgh == NULL)
    {
        fprintf(stderr, "%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* setup gridftp create */
    //globus_mutex_lock(&lookup_ctl_lock);
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->wrote_file = GLOBUS_FALSE;
    zgh->write_ctl_done = GLOBUS_FALSE;

    /* find the start and end offsets for this operation */
    start = file_starts[0];
    start_i = 0;
    end = file_starts[0] + file_sizes[0];
    end_i = 0;
    for( i = 1 ; i < file_count ; i++)
    {
        /* if the current index start is less than the least known start */
        if(file_starts[i] < (uint64_t)start)
        {
            start = file_starts[i];
            start_i = i;
        }

        /* if the current index end is greater than the greatest known end */
        if(file_starts[i] + file_sizes[i] > (uint64_t)end)
        {
            end = file_starts[i] + file_sizes[i];
            end_i = 0;
        }
    }

    /* get the file size */
    ret = globus_ftp_client_partial_put(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, start, end, write_ctl_cb, zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register partial put\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* IO */
    for(i = 0 ; i  < mem_count ; i++)
    {
        globus_bool_t eofvar = GLOBUS_FALSE;
        globus_off_t foff = file_starts[i];

        /* if this is the last element to write, pass EOF == TRUE */
        if(i == end_i)
            eofvar = GLOBUS_TRUE;

        /* register the write */
        ret = globus_ftp_client_register_write(&(zgh->gftpfh), (globus_byte_t *)mem_starts[i], (globus_size_t)mem_sizes[i], foff, eofvar, write_data_cb, (void *) zgh);
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not register write\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
            return ZFSERR_OTHER;
        }
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->write_ctl_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    globus_mutex_unlock(&(zgh->lock));
    //globus_mutex_unlock(&lookup_ctl_lock);

    return ZFS_OK;
}

static int zoidfs_gridftp_commit(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    /* all of our data is committed on write complete */
    return ZFS_OK;
}

static int zoidfs_gridftp_create(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      const zoidfs_sattr_t * UNUSED(attr),
                      zoidfs_handle_t * handle,
                      int * created,
                      zoidfs_op_hint_t * op_hint)
{
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;

    //globus_mutex_lock(&lookup_ctl_lock);
    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, handle);

    /* if the handle is already valid, just return */
    if(zgh->validHandle)
    {
        //globus_mutex_unlock(&lookup_ctl_lock);
        return ZFS_OK;
    }

    /* setup gridftp create */
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->file_created=GLOBUS_FALSE;
    zgh->create_done=GLOBUS_FALSE;
    zgh->create_wb_done=GLOBUS_FALSE;

    /* register put  and write a 0 byte file */
    fprintf(stderr, "%s : register create on %s, key = %lu\n", __func__, zgh->file_path, zgh->key);
    ret = globus_ftp_client_put(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, create_cb, (void *)zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register put\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        //globus_mutex_unlock(&lookup_ctl_lock);
        return ZFSERR_OTHER;
    }
    
    /* issue a write */
    globus_byte_t buf= (globus_byte_t)'\0';
    ret = globus_ftp_client_register_write(&(zgh->gftpfh), (globus_byte_t *)&buf, 0, (globus_off_t)0, GLOBUS_TRUE, create_data_cb, (void *)zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register write op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        //globus_mutex_unlock(&lookup_ctl_lock);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->create_done != GLOBUS_TRUE && zgh->create_wb_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    globus_mutex_unlock(&(zgh->lock));

    /* check if found the handle or not */
    if(zgh->file_created == GLOBUS_TRUE)
    {
        *created = 1;
        zgh->validHandle = 1;
    }
    else
    {
        *created = 0;
        err = ZFSERR_OTHER;
    }
    //globus_mutex_unlock(&lookup_ctl_lock);

    return err;
}

static int zoidfs_gridftp_remove(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, NULL);

    /* setup gridftp exist test */
    //globus_mutex_lock(&lookup_ctl_lock);
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->file_removed = GLOBUS_FALSE;
    zgh->remove_done = GLOBUS_FALSE;

    /* run delete */
    ret = globus_ftp_client_delete(&(zgh->gftpfh), zgh->file_path, GLOBUS_NULL, remove_cb, zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not run gftp delete op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->remove_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    globus_mutex_unlock(&(zgh->lock));
    //globus_mutex_unlock(&lookup_ctl_lock);

    /* if file was not removed, err */
    if(zgh->file_removed != GLOBUS_TRUE)
    {
        return ZFSERR_OTHER;
    }

    return ZFS_OK;
}

static int zoidfs_gridftp_rename(const zoidfs_handle_t * from_parent_handle,
                      const char * from_component_name,
                      const char * from_full_path,
                      const zoidfs_handle_t * to_parent_handle,
                      const char * to_component_name,
                      const char * to_full_path,
                      zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                      zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * fzgh = NULL;
    char t_path[1024];

    /* setup the connection and handles */
    fzgh = zoidfs_gridftp_setup_gridftp_handle(from_parent_handle, from_component_name, from_full_path, NULL);

    /* setup gridftp create */
    globus_mutex_init(&(fzgh->lock), GLOBUS_NULL);
    globus_cond_init(&(fzgh->cond), GLOBUS_NULL);
    fzgh->renamed_file=GLOBUS_FALSE;
    fzgh->rename_done=GLOBUS_FALSE;

    /* get the new path */
    zoidfs_gridftp_convert_filename(to_parent_handle, to_component_name, to_full_path, t_path);

    /* register put  and write a 0 byte file */
    ret = globus_ftp_client_move(&(fzgh->gftpfh), fzgh->file_path, t_path, &(fzgh->oattr), rename_cb, (void *)fzgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register move\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(fzgh->lock));
    while (fzgh->rename_done != GLOBUS_TRUE)
        globus_cond_wait(&(fzgh->cond), &(fzgh->lock));

    /* remove the handle from the tree and reinsert it under the new key / path */
    zoidfs_gridftp_handle_tree_remove( fzgh->key );
    memcpy(fzgh->file_path, t_path, strlen(t_path) + 1);
    fzgh->key = hash((void *)t_path, strlen(t_path) + 1, 0);
    zoidfs_gridftp_handle_tree_add(fzgh);

    globus_mutex_unlock(&(fzgh->lock));

    /* check if found the handle or not */
    if(fzgh->renamed_file != GLOBUS_TRUE)
    {
        return ZFSERR_OTHER;
    }
    return ZFS_OK;
}

/* hard links not supported by GridFTP */
static int zoidfs_gridftp_link(const zoidfs_handle_t * UNUSED(from_parent_handle),
                    const char * UNUSED(from_component_name),
                    const char * UNUSED(from_full_path),
                    const zoidfs_handle_t * UNUSED(to_parent_handle),
                    const char * UNUSED(to_component_name),
                    const char * UNUSED(to_full_path),
                    zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                    zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

/* symbolic links not supported by GridFTP */
static int zoidfs_gridftp_symlink(
    const zoidfs_handle_t * UNUSED(from_parent_handle),
    const char * UNUSED(from_component_name),
    const char * UNUSED(from_full_path),
    const zoidfs_handle_t * UNUSED(to_parent_handle),
    const char * UNUSED(to_component_name),
    const char * UNUSED(to_full_path),
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(from_parent_hint),
    zoidfs_cache_hint_t * UNUSED(to_parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zoidfs_gridftp_mkdir(
    const zoidfs_handle_t * parent_handle,
    const char * component_name,
    const char * full_path,
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, NULL);

    /* setup gridftp exist test */
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->dir_created = GLOBUS_FALSE;
    zgh->mkdir_done = GLOBUS_FALSE;

    /* run delete */
    ret = globus_ftp_client_mkdir(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), mkdir_cb, (void *)zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not run gftp delete op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->mkdir_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    globus_mutex_unlock(&(zgh->lock));

    /* if dir was not created, err */
    if(zgh->dir_created != GLOBUS_TRUE)
    {
        return ZFSERR_OTHER;
    }

    return ZFS_OK;
}

/* TODO */
static int zoidfs_gridftp_readdir(const zoidfs_handle_t * UNUSED(parent_handle),
                       zoidfs_dirent_cookie_t UNUSED(cookie),
                       size_t * UNUSED(entry_count),
                       zoidfs_dirent_t * UNUSED(entries),
                       uint32_t UNUSED(flags),
                       zoidfs_cache_hint_t * UNUSED(parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFSERR_NOTIMPL;
}

static int zoidfs_gridftp_resize(const zoidfs_handle_t * handle,
                      uint64_t size,
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hkey = 0;
    globus_off_t fsize = 0;

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);

    if(zgh == NULL)
    {
        fprintf(stderr, "%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* setup gridftp create */
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->file_resized = GLOBUS_FALSE;
    zgh->resize_done = GLOBUS_FALSE;
    zgh->resize_wb_done = GLOBUS_FALSE;

    /* get the file size */
    ret = globus_ftp_client_size(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &(fsize), resize_cb, zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register client size\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(zgh->lock));
    while (zgh->resize_done != GLOBUS_TRUE)
        globus_cond_wait(&(zgh->cond), &(zgh->lock));
    globus_mutex_unlock(&(zgh->lock));

    /* two cases:
     *  1) new size < old size
     *  2) new size > old size
     */

    if(fsize < (globus_off_t)size)
    {
        globus_byte_t buffer = (globus_byte_t)'\0';
        zgh->resize_done = GLOBUS_FALSE;
        
        ret = globus_ftp_client_partial_put(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, (globus_off_t)size - 1, (globus_off_t)size, resize_cb, (void *)zgh); 
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not register resize partial put\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
            return ZFSERR_OTHER;
        }

        ret = globus_ftp_client_register_write(&(zgh->gftpfh), (globus_byte_t *)&buffer, 1, (globus_off_t)(size - 1), GLOBUS_TRUE, resize_data_cb, (void *)zgh); 
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not register resize partial put\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
            return ZFSERR_OTHER;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(zgh->lock));
        while (zgh->resize_done != GLOBUS_TRUE)
            globus_cond_wait(&(zgh->cond), &(zgh->lock));
        globus_mutex_unlock(&(zgh->lock));
    } 
    else if(fsize > (globus_off_t)size)
    {
        zgh->resize_done = GLOBUS_FALSE;
        char * urlbak = NULL;

        /* make a backup of the old file */
        urlbak = (char *)malloc(sizeof(char) * strlen(zgh->file_path) + 5);
        sprintf(urlbak, "%s.bak", zgh->file_path);
        
        ret = globus_ftp_client_move(&(zgh->gftpfh), zgh->file_path, urlbak, &(zgh->oattr), resize_cb, (void *)zgh); 
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not register resize partial put\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
            return ZFSERR_OTHER;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(zgh->lock));
        while (zgh->resize_done != GLOBUS_TRUE)
            globus_cond_wait(&(zgh->cond), &(zgh->lock));
        globus_mutex_unlock(&(zgh->lock));

        /* transfer part of the file back to the orig file name */
        zgh->resize_done = GLOBUS_FALSE;
        ret = globus_ftp_client_partial_third_party_transfer(&(zgh->gftpfh), urlbak, &(zgh->oattr), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, 0, (globus_off_t)size, resize_cb, (void *)zgh);
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not register resize partial put\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
            return ZFSERR_OTHER;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(zgh->lock));
        while (zgh->resize_done != GLOBUS_TRUE && zgh->resize_wb_done != GLOBUS_TRUE)
            globus_cond_wait(&(zgh->cond), &(zgh->lock));
        globus_mutex_unlock(&(zgh->lock));

        /* delete the old file (in the bak url) */
        zgh->resize_done = GLOBUS_FALSE;
        ret = globus_ftp_client_delete(&(zgh->gftpfh), urlbak, &(zgh->oattr), resize_cb, (void *)zgh);
        if(ret != GLOBUS_SUCCESS)
        {
            fprintf(stderr, "%s : ERROR could not register resize partial put\n", __func__);
            fprintf(stderr, "%s : exit\n", __func__);
            return ZFSERR_OTHER;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(zgh->lock));
        while (zgh->resize_done != GLOBUS_TRUE)
            globus_cond_wait(&(zgh->cond), &(zgh->lock));
        globus_mutex_unlock(&(zgh->lock));

    }

    return ZFS_OK;
}

static int zoidfs_gridftp_resolve_path(const char * gridftp_path,
                            char * fs_path,
                            int fs_path_max,
                            zoidfs_handle_t * UNUSED(newhandle),
                            int * usenew)
{
    *usenew = 0;

    /*
     * If the fs_path buffer is available, identify the full path
     */
    if(fs_path)
    {
        strncpy(fs_path, gridftp_path, fs_path_max);
        fs_path[fs_path_max - 1] = '\0';
    }

    return ZFS_OK;
}

static int zoidfs_gridftp_init(void)
{
    globus_mutex_init(&lookup_ctl_lock, GLOBUS_NULL);
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);
    return ZFS_OK;
}

static int zoidfs_gridftp_finalize(void)
{
    /* cleanup the handle tree */
    zoidfs_gridftp_handle_tree_cleanup();

    globus_module_deactivate(GLOBUS_FTP_CLIENT_MODULE);
    return ZFS_OK;
}

static int zoidfs_gridftp_set_options(ConfigHandle c, SectionHandle s)
{
    int keysize = 0;
    char * keyvalue = NULL;
    int tkeyvalue = 0;

    /* get the connection caching variable */
    keysize = cf_getKey(c, s, "enableconnectioncache", NULL, 0);
    keyvalue = (char *)malloc(sizeof(char) * (keysize + 1));
    cf_getKey(c, s, "enableconnectioncache", keyvalue, keysize + 1);
    tkeyvalue = atoi(keyvalue);
    connection_caching_enabled = tkeyvalue ? 1 : 0;
    free(keyvalue);

    /* get the paralleism */
    keysize = cf_getKey(c, s, "parallelstreams", NULL, 0);
    keyvalue = (char *)malloc(sizeof(char) * (keysize + 1));
    cf_getKey(c, s, "parallelstreams", keyvalue, keysize + 1);
    parallel_streams = atoi(keyvalue);
    free(keyvalue);

    /* enable striping */
    keysize = cf_getKey(c, s, "enablestripes", NULL, 0);
    keyvalue = (char *)malloc(sizeof(char) * (keysize + 1));
    cf_getKey(c, s, "enablestripes", keyvalue, keysize + 1);
    tkeyvalue = atoi(keyvalue);
    stripes_enabled = tkeyvalue ? 1 : 0;
    free(keyvalue);

    /* get tcp buffer size */
    keysize = cf_getKey(c, s, "tcpbuffersize", NULL, 0);
    keyvalue = (char *)malloc(sizeof(char) * (keysize + 1));
    cf_getKey(c, s, "tcpbuffersize", keyvalue, keysize + 1);
    tcp_buffer_size = atoi(keyvalue);
    free(keyvalue);

    return ZFS_OK;
}

zint_handler_t gridftp_handler =
{
    zoidfs_gridftp_null,
    zoidfs_gridftp_getattr,
    zoidfs_gridftp_setattr,
    zoidfs_gridftp_lookup,
    zoidfs_gridftp_readlink,
    zoidfs_gridftp_read,
    zoidfs_gridftp_write,
    zoidfs_gridftp_commit,
    zoidfs_gridftp_create,
    zoidfs_gridftp_remove,
    zoidfs_gridftp_rename,
    zoidfs_gridftp_link,
    zoidfs_gridftp_symlink,
    zoidfs_gridftp_mkdir,
    zoidfs_gridftp_readdir,
    zoidfs_gridftp_resize,
    zoidfs_gridftp_resolve_path,
    zoidfs_gridftp_init,
    zoidfs_gridftp_finalize,
    zoidfs_gridftp_set_options
};
