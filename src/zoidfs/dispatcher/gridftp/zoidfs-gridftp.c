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

#ifdef ZOIDFS_GRIDFTP_DEBUG_ENABLED
#define ZOIDFS_GRIDFTP_DEBUG(...) \
    do { \
        char __buffer[4096]; \
        sprintf(__buffer, ##__VA_ARGS__); \
        fprintf(stderr, "%s %s, ZOIDFS GRIDFTP DISPATCHER - DEBUG %s() %s:%i : %s\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__, __buffer); \
    }while(0)

#define ZOIDFS_GRIDFTP_INFO(...) \
    do { \
        char __buffer[4096]; \
        sprintf(__buffer, ##__VA_ARGS__); \
        fprintf(stderr, "%s %s, ZOIDFS GRIDFTP DISPATCHER - INFO %s() %s:%i : %s\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__, __buffer); \
    }while(0)

#define ZOIDFS_GRIDFTP_PERROR(...) \
    do { \
        char __buffer[4096]; \
        char __ebuffer[2048]; \
        strerror_r(errno, __ebuffer, 2048); \
        sprintf(__buffer, ##__VA_ARGS__); \
        fprintf(stderr, "%s %s, ZOIDFS GRIDFTP DISPATCHER - ERROR %s() %s:%i : %s, %s\n", __DATE__, __TIME__, __func__, __FILE__, __LINE__, __buffer, __ebuffer); \
    }while(0)
#else
#define ZOIDFS_GRIDFTP_DEBUG(...)
#define ZOIDFS_GRIDFTP_INFO(...)
#define ZOIDFS_GRIDFTP_PERROR(...) 
#endif

/* driver configuration variables */
static int connection_caching_enabled = 0;
static int stripes_enabled = 0;
static int tcp_buffer_size = 0;
static int parallel_streams = 0; 

/* lock for ctl ch ops */
static globus_mutex_t zoidfs_gridftp_ctl_ch_lock;

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
    globus_bool_t file_created_wb;
    globus_bool_t getattr_done;

    /* data amounts */
    globus_size_t bytes_written;
    globus_size_t bytes_read;

} zoidfs_gridftp_op_t;

static zoidfs_gridftp_op_t * zoidfs_gridftp_op_create()
{
    /* create the op */
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *) malloc(sizeof(zoidfs_gridftp_op_t));

    /* init the locks */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);

    /* init handle flags to false */
    op->file_exists = GLOBUS_FALSE;
    op->exists_done = GLOBUS_FALSE;
    op->file_created = GLOBUS_FALSE;
    op->create_done = GLOBUS_FALSE;
    op->create_wb_done = GLOBUS_FALSE;
    op->file_removed = GLOBUS_FALSE;
    op->remove_done = GLOBUS_FALSE;
    op->dir_created = GLOBUS_FALSE;
    op->mkdir_done = GLOBUS_FALSE;
    op->file_resized = GLOBUS_FALSE;
    op->resize_done = GLOBUS_FALSE;
    op->resize_wb_done = GLOBUS_FALSE;
    op->wrote_file = GLOBUS_FALSE;
    op->write_ctl_done = GLOBUS_FALSE;
    op->read_file = GLOBUS_FALSE;
    op->read_ctl_done = GLOBUS_FALSE;
    op->renamed_file = GLOBUS_FALSE;
    op->rename_done = GLOBUS_FALSE;
    op->getattr_done = GLOBUS_FALSE;
    op->file_created_wb = GLOBUS_FALSE;
    op->getattr_done = GLOBUS_FALSE;

    op->bytes_written = 0;
    op->bytes_read = 0;

    return op;
}

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

    /* is this a valid handle */
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
    h->validHandle = 0;

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
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not destroy handle\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        }

        /* cleanup handle attr info */
        ret = globus_ftp_client_handleattr_destroy(&(hh->hattr));
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not destroy handle attr\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        }

        /* cleanup op attr info */
        ret = globus_ftp_client_operationattr_destroy(&(hh->oattr));
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not destroy operation attr\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
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
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->file_exists = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    op->exists_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
    ZOIDFS_GRIDFTP_PERROR("%s : signal cond\n", __func__);
}

static void remove_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->file_removed = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    op->remove_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void mkdir_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->dir_created = GLOBUS_TRUE;
    }

    /* update done flag and signal */
    op->mkdir_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void rename_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->renamed_file = GLOBUS_TRUE;
    }

    /* update done flag and signal */
    op->rename_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void create_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->file_created = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    op->create_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void create_data_cb(void * myargs, globus_ftp_client_handle_t * handle, globus_object_t * error,
    globus_byte_t * buffer, globus_size_t length, globus_off_t offset, globus_bool_t UNUSED(eof))
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
   
    globus_ftp_client_register_write(handle, buffer, length, offset, GLOBUS_FALSE, create_data_cb, myargs);
    op->file_created_wb = GLOBUS_TRUE;
  
    /* update done flag and signal */ 
    op->create_wb_done = GLOBUS_TRUE;
}

static void getattr_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    
    /* update done flag and signal */
    op->getattr_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void resize_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->file_resized = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    op->resize_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void resize_data_cb(void * myargs, globus_ftp_client_handle_t * handle, globus_object_t * error,
    globus_byte_t * buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
            globus_ftp_client_register_write(handle, buffer, length, offset, eof, resize_data_cb, myargs);
    }
  
    /* update done flag and signal */ 
    op->resize_wb_done = GLOBUS_TRUE;
}

static void write_ctl_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->wrote_file = GLOBUS_TRUE;
    }
    op->write_ctl_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void write_data_cb(void * myargs, globus_ftp_client_handle_t * handle, globus_object_t * error,
                                   globus_byte_t * buffer, globus_size_t length, globus_off_t offset,
                                   globus_bool_t eof)
{
    globus_size_t * bytes_written = NULL;
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;

    bytes_written = &(op->bytes_written);
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }

    *bytes_written += length;
    if(!eof)
    {
        globus_ftp_client_register_write(handle, buffer, length, offset, GLOBUS_TRUE, write_data_cb, (void *)bytes_written);
    }
}

static void read_ctl_cb(void * myargs, globus_ftp_client_handle_t * UNUSED(handle), globus_object_t * error)
{
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
    }
    else
    {
        op->read_file = GLOBUS_TRUE;
    }
    op->read_ctl_done = GLOBUS_TRUE;
    globus_cond_signal(&(op->cond));
}

static void read_data_cb(void * myargs, globus_ftp_client_handle_t * handle, globus_object_t * error,
                                   globus_byte_t * buffer, globus_size_t length, globus_off_t UNUSED(offset),
                                   globus_bool_t eof)
{
    globus_size_t * bytes_read = NULL;
    zoidfs_gridftp_op_t * op = (zoidfs_gridftp_op_t *)myargs;

    bytes_read = &(op->bytes_read);
    if(error)
    {
#ifdef ZOIDFS_GRIDFTP_DEBUG
        ZOIDFS_GRIDFTP_PERROR("%s\n", globus_object_printable_to_string(error));
#endif
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

#if 0
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
#endif

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
    /* if the full path was given, parse the path into the file path, host, port, and method */
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
    /* else, get the parent handle and append the component name to the path */
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

    /* get the filename / path */
    zoidfs_gridftp_convert_filename(parent_handle, component_name, full_path, &gftp_path[0]);

    /* we have the full path, break it into tokens */
    if(full_path)
    {
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
            hv = hash((void *)gftp_path, strlen(gftp_path) + 1, 0);
            zgh = zoidfs_gridftp_handle_tree_find(hv);
        }
    }

    /* handle was found, return that handle */
    if(zgh != NULL)
    {
        /* make a copy of the handle if a valid handle pointer was provided */
        if(handle != NULL)
        {
            memcpy(handle, &(zgh->zfs_handle), sizeof(zoidfs_handle_t));
        }
        return zgh;
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
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not init handle\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        return NULL;
    }

    /* init the operation */
    ret = globus_ftp_client_operationattr_init(&(zgh->oattr));
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not init op\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        return NULL;
    }

    /* enable gridftp connection caching */
    if(connection_caching_enabled)
    {
        ret = globus_ftp_client_handleattr_set_cache_all(&(zgh->hattr), GLOBUS_TRUE);
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not cache connection\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
            return NULL;
        }
    }

    /* set the authorization params */
    ret = globus_ftp_client_operationattr_set_authorization(&(zgh->oattr), GSS_C_NO_CREDENTIAL, "copej", "", "copej", "");
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not set authorization\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        return NULL;
    }

    /* handle init */
    ret = globus_ftp_client_handle_init(&(zgh->gftpfh), &(zgh->hattr));
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR reinint handle, %s\n", __func__, globus_object_printable_to_string(globus_error_get(ret)));
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
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
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hkey = 0;
    globus_off_t fsize = 0;
    globus_abstime_t mtime;
    globus_size_t stat_buffer_size = 4096;
    globus_byte_t * stat_buffer;
    zoidfs_gridftp_op_t * op = NULL;

    stat_buffer = (globus_byte_t *)malloc(stat_buffer_size * sizeof(globus_byte_t));

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);
    op = zoidfs_gridftp_op_create();

    if(zgh == NULL)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->getattr_done = GLOBUS_FALSE;

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* get the file size */
    ret = globus_ftp_client_size(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &(fsize), getattr_cb, op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register client size\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->getattr_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    attr->size = (uint64_t)fsize;
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* get the file modification time */
    op->getattr_done = GLOBUS_FALSE;
    ret = globus_ftp_client_modification_time(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &mtime, getattr_cb, op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register client size\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->getattr_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));

    /* update all of the times to the mtime */
    attr->mtime.seconds = mtime.tv_sec; 
    attr->mtime.nseconds = 0; 
    attr->atime.seconds = mtime.tv_sec; 
    attr->atime.nseconds = 0; 
    attr->ctime.seconds = mtime.tv_sec; 
    attr->ctime.nseconds = 0;
 
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* get the file stat info */
    op->getattr_done = GLOBUS_FALSE;
    ret = globus_ftp_client_stat(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &stat_buffer, &stat_buffer_size, getattr_cb, op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register client size\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->getattr_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* parse the stat info */
    char * statstr = NULL;
    char * statsaveptr = NULL;
    unsigned int j = 0;
    char * token = NULL;
    for (j = 1, statstr = (char *)stat_buffer; ; j++, statstr = NULL)
    {
        token = strtok_r(statstr, " ", &statsaveptr);
        if (token == NULL)
            break;

        /* parse the file mode, perms */
        if(j == 1)
        {
            unsigned int k = 0;
            for(k = 0 ; k < strlen(token) ; k++)
            {
                if(token[k] != '-')
                {
                    attr->mode += 1;
                }
                if( k < strlen(token) - 1)
                {
                    attr->mode = attr->mode << 1;
                }
            }
        }
        /* parse the nlinks */
        else if(j == 2)
        {
            attr->nlink = atoi(token);
        }
        /* parse the owner */
        else if(j == 4)
        {
            /* not possible... no way to get the uid on the remote host */
        }
        /* parse the group */
        else if(j == 4)
        {
            /* not possible... no way to get the gid on the remote host */
        }
        /* parse the file size */
        else if(j == 5)
        {
            /* we already did this... skip */
        }
        /* parse the timestamp */
        else if(j == 6 || j ==7 || j == 8)
        {
            /* we already did this... skip */
        }
        /* parse the file name */ 
        else
        {
            /* ignore the file name... we have it in our internal handle */
        }
    }

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(stat_buffer)
    {
        free(stat_buffer);
        stat_buffer = NULL;
    }

    if(op)
    {
        free(op);
        op = NULL;
    }

    return err;
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
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    zoidfs_gridftp_op_t * op = NULL;

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, handle);

    /* if the handle is already valid, just return */
    if(zgh->validHandle)
    {
        return ZFS_OK;
    }

    op = zoidfs_gridftp_op_create();

    /* setup gridftp exist test */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->file_exists = GLOBUS_FALSE;
    op->exists_done = GLOBUS_FALSE;

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* run exist test */
    ret = globus_ftp_client_exists(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), exists_cb, (void *)op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not run gftp exist op\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->exists_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    if(op->file_exists != GLOBUS_TRUE)
    {
        zgh->validHandle = 1;
        err = ZFSERR_EXIST;
        goto cleanup;
    }

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }
 
    return err;
}

/* not supported */
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
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hkey = 0;
    globus_off_t start = 0;
    globus_off_t end = 0;
    size_t i = 0;
    size_t start_i = 0;
    size_t end_i = 0;
    char * gftp_buffer = NULL;
    zoidfs_gridftp_op_t * op = NULL;

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);

    if(zgh == NULL)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    op = zoidfs_gridftp_op_create();

    /* setup gridftp create */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->read_file = GLOBUS_FALSE;
    op->read_ctl_done = GLOBUS_FALSE;

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

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* start a partial get */
    ret = globus_ftp_client_partial_get(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, start, end, read_ctl_cb, op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register partial get\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* allocate the read buffer */
    gftp_buffer = (char *) malloc(sizeof(char) * (end - start));

    /* register the read */
    ret = globus_ftp_client_register_read(&(zgh->gftpfh), (globus_byte_t *)gftp_buffer, (globus_size_t)(end - start), read_data_cb, (void *) op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register read\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->read_ctl_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* copy the buffer segments into the user args */
    for(i = 0 ; i < mem_count ; i++)
    {
        const uint64_t foff = file_starts[i] - start;
        memcpy(mem_starts[i], &(gftp_buffer[foff]), mem_sizes[i]);
    }   

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }

    if(gftp_buffer)
    {
        free(gftp_buffer);
    }

    return err;
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
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hkey = 0;
    globus_off_t start = 0;
    globus_off_t end = 0;
    size_t i = 0;
    size_t start_i = 0;
    size_t end_i = 0;
    zoidfs_gridftp_op_t * op = NULL;

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);

    if(zgh == NULL)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    op = zoidfs_gridftp_op_create();

    /* setup gridftp create */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->wrote_file = GLOBUS_FALSE;
    op->write_ctl_done = GLOBUS_FALSE;

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

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* start the partial put */
    ret = globus_ftp_client_partial_put(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, start, end, write_ctl_cb, op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register partial put\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
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
        ret = globus_ftp_client_register_write(&(zgh->gftpfh), (globus_byte_t *)mem_starts[i], (globus_size_t)mem_sizes[i], foff, eofvar, write_data_cb, (void *) op);
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register write\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
            err = ZFSERR_OTHER;
            goto cleanup;
        }
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->write_ctl_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }

    return err;
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
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    zoidfs_gridftp_op_t * op = NULL;

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, handle);

    /* if the handle is already valid, just return */
    if(zgh->validHandle)
    {
        return ZFS_OK;
    }

    op = zoidfs_gridftp_op_create();

    /* setup gridftp create */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->file_created = GLOBUS_FALSE;
    op->create_done = GLOBUS_FALSE;
    op->create_wb_done = GLOBUS_FALSE;

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* start a partial put */
    op->create_done = GLOBUS_FALSE;
    ret = globus_ftp_client_partial_put(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, 0, 0, create_cb, (void *)op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register put\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }
    
    /* issue a write */
    op->file_created_wb = GLOBUS_FALSE;
    globus_byte_t buf= (globus_byte_t)'\0';
    ret = globus_ftp_client_register_write(&(zgh->gftpfh), (globus_byte_t *)&buf, 0, (globus_off_t)0, GLOBUS_TRUE, create_data_cb, (void *)op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register write op\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->create_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* check if found the handle or not */
    if(op->file_created == GLOBUS_TRUE)
    {
        *created = 1;
        zgh->validHandle = 1;
    }
    else
    {
        *created = 0;
        err = ZFSERR_OTHER;
    }

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }

    return err;
}

static int zoidfs_gridftp_remove(const zoidfs_handle_t * parent_handle,
                      const char * component_name,
                      const char * full_path,
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    zoidfs_gridftp_op_t * op = NULL;

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, NULL);

    op = zoidfs_gridftp_op_create();

    /* setup gridftp exist test */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->file_removed = GLOBUS_FALSE;
    op->remove_done = GLOBUS_FALSE;

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* run delete */
    ret = globus_ftp_client_delete(&(zgh->gftpfh), zgh->file_path, GLOBUS_NULL, remove_cb, op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not run gftp delete op\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->remove_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* if file was not removed, err */
    if(op->file_removed != GLOBUS_TRUE)
    {
        err = ZFSERR_OTHER;
    }

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }

    return err;
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
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * fzgh = NULL;
    zoidfs_gridftp_op_t * op = NULL;
    char t_path[1024];

    /* setup the connection and handles */
    fzgh = zoidfs_gridftp_setup_gridftp_handle(from_parent_handle, from_component_name, from_full_path, NULL);
    op = zoidfs_gridftp_op_create();
    
    /* setup gridftp create */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->renamed_file = GLOBUS_FALSE;
    op->rename_done = GLOBUS_FALSE;

    /* get the new path */
    zoidfs_gridftp_convert_filename(to_parent_handle, to_component_name, to_full_path, t_path);

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* start the move */
    ret = globus_ftp_client_move(&(fzgh->gftpfh), fzgh->file_path, t_path, &(fzgh->oattr), rename_cb, (void *)fzgh);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register move\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->rename_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));

    /* remove the handle from the tree and reinsert it under the new key / path */
    zoidfs_gridftp_handle_tree_remove( fzgh->key );
    memcpy(fzgh->file_path, t_path, strlen(t_path) + 1);
    fzgh->key = hash((void *)t_path, strlen(t_path) + 1, 0);
    zoidfs_gridftp_handle_tree_add(fzgh);

    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* check if found the handle or not */
    if(op->renamed_file != GLOBUS_TRUE)
    {
        err = ZFSERR_OTHER;
    }

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }

    return err;
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
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    zoidfs_gridftp_op_t * op = NULL;

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, NULL);
    op = zoidfs_gridftp_op_create();

    /* setup gridftp exist test */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->dir_created = GLOBUS_FALSE;
    op->mkdir_done = GLOBUS_FALSE;

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* run mkdir */
    ret = globus_ftp_client_mkdir(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), mkdir_cb, (void *)op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not run gftp delete op\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->mkdir_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* if dir was not created, err */
    if(op->dir_created != GLOBUS_TRUE)
    {
        err = ZFSERR_OTHER;
    }

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }

    return err;
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
    int err = ZFS_OK;
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;
    zoidfs_gridftp_op_t * op = NULL;
    uint64_t hkey = 0;
    globus_off_t fsize = 0;

    /* get the gridftp handle from the zoidfs handle */
    hkey = zoidfs_gridftp_zfs_handle_get(handle);
    zgh = zoidfs_gridftp_handle_tree_find(hkey);

    if(zgh == NULL)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not find the handle associated with key = %lu\n", __func__, hkey);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    op = zoidfs_gridftp_op_create();

    /* setup gridftp create */
    globus_mutex_init(&(op->lock), GLOBUS_NULL);
    globus_cond_init(&(op->cond), GLOBUS_NULL);
    op->file_resized = GLOBUS_FALSE;
    op->resize_done = GLOBUS_FALSE;
    op->resize_wb_done = GLOBUS_FALSE;

    /* lock before ctl ch op */
    globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

    /* get the file size */
    ret = globus_ftp_client_size(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), &(fsize), resize_cb, op);
    if(ret != GLOBUS_SUCCESS)
    {
        ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register client size\n", __func__);
        ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
        err = ZFSERR_OTHER;
        goto cleanup;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&(op->lock));
    while (op->resize_done != GLOBUS_TRUE)
        globus_cond_wait(&(op->cond), &(op->lock));
    globus_mutex_unlock(&(op->lock));

    /* ctl ch op done, unlock */
    globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

    /* two cases:
     *  1) new size < old size
     *  2) new size > old size
     */

    /* for the old size < new size case 
     *  1) do a partial put on the file for byte range size - 1, size 
     *  2) write 1 byte at offset size - 1
     */ 
    if(fsize < (globus_off_t)size)
    {
        globus_byte_t buffer = (globus_byte_t)'\0';
        op->resize_done = GLOBUS_FALSE;
        
        /* lock before ctl ch op */
        globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

        /* start a partial put */
        ret = globus_ftp_client_partial_put(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, (globus_off_t)size - 1, (globus_off_t)size, resize_cb, (void *)op); 
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register resize partial put\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
            err = ZFSERR_OTHER;
            goto cleanup;
        }

        /* register a write */
        ret = globus_ftp_client_register_write(&(zgh->gftpfh), (globus_byte_t *)&buffer, 1, (globus_off_t)(size - 1), GLOBUS_TRUE, resize_data_cb, (void *)op); 
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register resize partial put\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
            err = ZFSERR_OTHER;
            goto cleanup;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(op->lock));
        while (op->resize_done != GLOBUS_TRUE)
            globus_cond_wait(&(op->cond), &(op->lock));
        globus_mutex_unlock(&(op->lock));

        /* ctl ch op done, unlock */
        globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);
    } 
    /* for the old size > new size case 
     *  1) move original file to backup file 
     *  2) copy (through 3rd party transfer) size data from 0, size - 1
     *   from backup file to new file 
     *  2) delete backup file  
     */ 
    else if(fsize > (globus_off_t)size)
    {
        op->resize_done = GLOBUS_FALSE;
        char * urlbak = NULL;

        /* make a backup of the old file */
        urlbak = (char *)malloc(sizeof(char) * strlen(zgh->file_path) + 5);
        sprintf(urlbak, "%s.bak", zgh->file_path);
        
        /* lock before ctl ch op */
        globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

        /* start a move */
        ret = globus_ftp_client_move(&(zgh->gftpfh), zgh->file_path, urlbak, &(zgh->oattr), resize_cb, (void *)op); 
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register resize partial put\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
            err = ZFSERR_OTHER;
            goto cleanup;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(op->lock));
        while (op->resize_done != GLOBUS_TRUE)
            globus_cond_wait(&(op->cond), &(op->lock));
        globus_mutex_unlock(&(op->lock));

        /* ctl ch op done, unlock */
        globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

        /* transfer part of the file back to the orig file name */
        op->resize_done = GLOBUS_FALSE;
        
        /* lock before ctl ch op */
        globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

        /* regsiter partial 3rd party transfer */
        ret = globus_ftp_client_partial_third_party_transfer(&(zgh->gftpfh), urlbak, &(zgh->oattr), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, 0, (globus_off_t)size, resize_cb, (void *)zgh);
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register resize partial put\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
            err = ZFSERR_OTHER;
            goto cleanup;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(op->lock));
        while (op->resize_done != GLOBUS_TRUE && op->resize_wb_done != GLOBUS_TRUE)
            globus_cond_wait(&(op->cond), &(op->lock));
        globus_mutex_unlock(&(op->lock));

        /* ctl ch op done, unlock */
        globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);

        /* delete the old file (in the bak url) */
        op->resize_done = GLOBUS_FALSE;
        
        /* lock before ctl ch op */
        globus_mutex_lock(&zoidfs_gridftp_ctl_ch_lock);

        /* run the delete */
        ret = globus_ftp_client_delete(&(zgh->gftpfh), urlbak, &(zgh->oattr), resize_cb, (void *)op);
        if(ret != GLOBUS_SUCCESS)
        {
            ZOIDFS_GRIDFTP_PERROR("%s : ERROR could not register resize partial put\n", __func__);
            ZOIDFS_GRIDFTP_PERROR("%s : exit\n", __func__);
            err = ZFSERR_OTHER;
            goto cleanup;
        }

        /* wait for the callback to finish */
        globus_mutex_lock(&(op->lock));
        while (op->resize_done != GLOBUS_TRUE)
            globus_cond_wait(&(op->cond), &(op->lock));
        globus_mutex_unlock(&(op->lock));

        /* ctl ch op done, unlock */
        globus_mutex_unlock(&zoidfs_gridftp_ctl_ch_lock);
    }

cleanup:
    globus_cond_destroy(&(op->cond));
    globus_mutex_destroy(&(op->lock));
    if(op)
    {
        free(op);
        op = NULL;
    }

    return err;
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
    /* init the gftp lib */
    globus_module_activate(GLOBUS_FTP_CLIENT_MODULE);

    /* setup the ctl ch lock */
    globus_mutex_init(&zoidfs_gridftp_ctl_ch_lock, GLOBUS_NULL);
    return ZFS_OK;
}

static int zoidfs_gridftp_finalize(void)
{
    /* cleanup the handle tree */
    zoidfs_gridftp_handle_tree_cleanup();

    /* shutdown the gftp lib */
    globus_module_deactivate(GLOBUS_FTP_CLIENT_MODULE);

    /* setup the ctl ch lock */
    globus_mutex_destroy(&zoidfs_gridftp_ctl_ch_lock);

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
