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
 
} zoidfs_gridftp_handle_t;

/* handle create and destroy funcs */
static zoidfs_gridftp_handle_t * zoidfs_gridftp_handle_create()
{
    zoidfs_gridftp_handle_t * h = NULL;

    /* allocate a new handle */
    h = (zoidfs_gridftp_handle_t *)malloc(sizeof(zoidfs_gridftp_handle_t));

    h->key = 0;
    h->file_path = NULL;

    /* init the locks */
    globus_mutex_init(&(h->lock), GLOBUS_NULL);
    globus_cond_init(&(h->cond), GLOBUS_NULL);

    /* init handle flags to false */    
    h->file_exists = GLOBUS_FALSE;
    h->exists_done = GLOBUS_FALSE;
    h->file_created = GLOBUS_FALSE;
    h->create_done = GLOBUS_FALSE;
    h->create_wb_done = GLOBUS_FALSE;

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

static zoidfs_gridftp_handle_t * zoidfs_gridftp_setup_gridftp_handle(const zoidfs_handle_t * parent_handle,
            const char * component_name,
            const char * full_path,
            zoidfs_handle_t * handle)
{
    globus_result_t ret;
    char gftp_path[1024];
    zoidfs_gridftp_handle_t * zgh = NULL;
    uint64_t hv = 0;

    /* right now, use ftp */
    sprintf(gftp_path, "ftp://127.0.0.1:2811%s", full_path);

    hv = hash((void *)gftp_path, strlen(gftp_path) + 1, 0);
    zgh = zoidfs_gridftp_handle_tree_find(hv);

    if(zgh != NULL)
    {
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
        fprintf(stderr, "%s : ERROR could not init handle\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return NULL;
    }

    /* init the operation. @TODO fix the handle */
    ret = globus_ftp_client_operationattr_init(&(zgh->oattr));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not init op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return NULL;
    }

    /* enable gridftp connection caching */
    ret = globus_ftp_client_handleattr_set_cache_all(&(zgh->hattr), GLOBUS_TRUE);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not cache connection\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return NULL;
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

static int zoidfs_gridftp_null(void)
{
    return ZFS_OK;
}

static int zoidfs_gridftp_getattr(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_attr_t * UNUSED(attr),
                       zoidfs_op_hint_t * UNUSED(hint))
{
    return ZFS_OK;
}

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

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, handle);

    /* setup gridftp exist test */
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->file_exists = GLOBUS_FALSE;
    zgh->exists_done = GLOBUS_FALSE;

    /* run exist test */
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
   
    return ZFS_OK;
}

static int zoidfs_gridftp_readlink(const zoidfs_handle_t * UNUSED(handle),
                        char * UNUSED(buffer),
                        size_t UNUSED(buffer_length),
                        zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_read(const zoidfs_handle_t * UNUSED(handle),
                    size_t UNUSED(mem_count),
                    void * UNUSED(mem_starts[]),
                    const size_t UNUSED(mem_sizes[]),
                    size_t UNUSED(file_count),
                    const uint64_t UNUSED(file_starts[]),
                    uint64_t UNUSED(file_sizes[]),
                    zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_write(const zoidfs_handle_t * UNUSED(handle),
                     size_t UNUSED(mem_count),
                     const void * UNUSED(mem_starts[]),
                     const size_t UNUSED(mem_sizes[]),
                     size_t UNUSED(file_count),
                     const uint64_t UNUSED(file_starts[]),
                     uint64_t UNUSED(file_sizes[]),
                     zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_commit(const zoidfs_handle_t * UNUSED(handle),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
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
    globus_result_t ret;
    zoidfs_gridftp_handle_t * zgh = NULL;

    /* setup the connection and handles */
    zgh = zoidfs_gridftp_setup_gridftp_handle(parent_handle, component_name, full_path, handle);

    /* setup gridftp create */
    globus_mutex_init(&(zgh->lock), GLOBUS_NULL);
    globus_cond_init(&(zgh->cond), GLOBUS_NULL);
    zgh->file_created=GLOBUS_FALSE;
    zgh->create_done=GLOBUS_FALSE;
    zgh->create_wb_done=GLOBUS_FALSE;

    /* register put  and write a 0 byte file */
    ret = globus_ftp_client_put(&(zgh->gftpfh), zgh->file_path, &(zgh->oattr), GLOBUS_NULL, create_cb, (void *)zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register put\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }
    
    /* issue a write */
    globus_byte_t buf= (globus_byte_t)'\0';
    ret = globus_ftp_client_register_write(&(zgh->gftpfh), (globus_byte_t *)&buf, 0, (globus_off_t)0, GLOBUS_TRUE, create_data_cb, (void *)zgh);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register write op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
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
    }
    else
    {
        *created = 0;
    }

    return ZFS_OK;
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

    return ZFS_OK;
}

static int zoidfs_gridftp_rename(const zoidfs_handle_t * UNUSED(from_parent_handle),
                      const char * UNUSED(from_component_name),
                      const char * UNUSED(from_full_path),
                      const zoidfs_handle_t * UNUSED(to_parent_handle),
                      const char * UNUSED(to_component_name),
                      const char * UNUSED(to_full_path),
                      zoidfs_cache_hint_t * UNUSED(from_parent_hint),
                      zoidfs_cache_hint_t * UNUSED(to_parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

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
    return ZFS_OK;
}

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
    return ZFS_OK;
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

    return ZFS_OK;
}

static int zoidfs_gridftp_readdir(const zoidfs_handle_t * UNUSED(parent_handle),
                       zoidfs_dirent_cookie_t UNUSED(cookie),
                       size_t * UNUSED(entry_count),
                       zoidfs_dirent_t * UNUSED(entries),
                       uint32_t UNUSED(flags),
                       zoidfs_cache_hint_t * UNUSED(parent_hint),
                       zoidfs_op_hint_t * UNUSED(op_hint))
{
    return ZFS_OK;
}

static int zoidfs_gridftp_resize(const zoidfs_handle_t * UNUSED(handle),
                      uint64_t UNUSED(size),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
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

static int zoidfs_gridftp_set_options(ConfigHandle UNUSED(c), SectionHandle UNUSED(s))
{
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
