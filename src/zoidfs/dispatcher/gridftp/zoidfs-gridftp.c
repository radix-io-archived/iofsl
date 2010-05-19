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

#include "fcache.h"

#include "globus_ftp_client.h"

/* zoidfs gridftp handle */
typedef struct zoidfs_gridftp_handle
{
    /* gridftp client handles and attrs */
    globus_ftp_client_handle_t h;
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
} zoidfs_gridftp_handle_t;

static fcache_handle fcache;

static int num_gridftp_handles = 0;
#ifndef ZOIDFS_GRIDFTP_HANDLES_MAX
#define ZOIDFS_GRIDFTP_HANDLES_MAX 200
#endif
static globus_ftp_client_handle_t gridftp_fh[ZOIDFS_GRIDFTP_HANDLES_MAX];
static globus_ftp_client_operationattr_t oattr[ZOIDFS_GRIDFTP_HANDLES_MAX];

static globus_mutex_t lock;
static globus_cond_t cond;
static globus_bool_t file_exists;
static globus_bool_t exists_done;

static void exists_cb(void *myargs, globus_ftp_client_handle_t *handle, globus_object_t *error)
{
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        file_exists = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    exists_done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
}

static globus_bool_t file_created;
static globus_bool_t create_done;
static globus_bool_t create_wb_done;
static void create_cb(void *myargs, globus_ftp_client_handle_t *handle, globus_object_t *error)
{
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        file_created = GLOBUS_TRUE;
    }
    
    /* update done flag and signal */
    create_done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
}

static void create_data_cb(void *myargs, globus_ftp_client_handle_t *handle, globus_object_t *error,
    globus_byte_t *buffer, globus_size_t length, globus_off_t offset, globus_bool_t eof)
{
    if(error)
    {
        /* fprintf(stderr, "%s\n", globus_object_printable_to_string(error)); */
    }
    else
    {
        file_created = GLOBUS_TRUE;
    }
  
    /* update done flag and signal */ 
    create_wb_done = GLOBUS_TRUE;
    globus_cond_signal(&cond);
}

int zoidfs_gridftp_filename_to_handle(const zoidfs_gridftp_handle_t * zgh, const char * buf, zoidfs_handle_t * h)
{
    unsigned int ofs = 0;

    SHA1Context con;
    SHA1Reset(&con);
    SHA1Input(&con, (const unsigned char *) buf, strlen(buf));
    SHA1Result(&con);

    memset(h->data, 0, sizeof(h->data));

    /* we have 5 bytes of SHA-1; skip reserved part */
    ofs += 4;  /* skip reserved part */

    memcpy(&h->data[ofs], &con.Message_Digest[0], sizeof (con.Message_Digest));
    ofs += sizeof (con.Message_Digest);

    memcpy(&h->data[ofs], zgh, sizeof(zoidfs_gridftp_handle_t *));
    ofs += sizeof (zoidfs_gridftp_handle_t *);

    assert(ofs < sizeof (h->data));

    return 1;
}

static int zoidfs_gridftp_handle_to_filename (const zoidfs_handle_t * handle,
      char * buf, int bufsize)
{
    assert(bufsize >= ZOIDFS_PATH_MAX);
    return filename_lookup(fcache, handle, buf, bufsize);
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
    globus_ftp_client_handleattr_t hattr;
    char gftp_path[1024];

    /* right now, use ftp */
    sprintf(gftp_path, "ftp://127.0.0.1:2811%s", full_path);

    /* init the handle attr */
    ret = globus_ftp_client_handleattr_init(&hattr);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not init handle\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* init the operation. @TODO fix the handle */
    ret = globus_ftp_client_operationattr_init(&(oattr[0]));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not init op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* enable gridftp connection caching */
    ret = globus_ftp_client_handleattr_set_cache_all(&hattr, GLOBUS_TRUE);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not cache connection\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* set the authorization params */
    ret = globus_ftp_client_operationattr_set_authorization(&(oattr[0]), GSS_C_NO_CREDENTIAL, "copej", "", "copej", "");
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not set authorization\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }
    
    /* handle init */
    ret = globus_ftp_client_handle_init(&(gridftp_fh[0]), &hattr);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR reinint handle, %s\n", __func__, globus_object_printable_to_string(globus_error_get(ret)));
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* setup gridftp exist test */
    globus_mutex_init(&lock, GLOBUS_NULL);
    globus_cond_init(&cond, GLOBUS_NULL);
    file_exists = GLOBUS_FALSE;
    exists_done = GLOBUS_FALSE;

    /* run exist test */
    ret = globus_ftp_client_exists(&(gridftp_fh[0]), gftp_path, &(oattr[0]), exists_cb, GLOBUS_NULL);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not run gftp exist op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&lock);
    while (exists_done != GLOBUS_TRUE)
        globus_cond_wait(&cond, &lock);
    globus_mutex_unlock(&lock);

    /* cleanup handle info */
    ret = globus_ftp_client_handle_destroy(&(gridftp_fh[0]));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not destroy handle\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }
    
    /* cleanup handle attr info */
    ret = globus_ftp_client_handleattr_destroy(&hattr);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not destroy handle attr\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }
    
    /* cleanup op attr info */
    ret = globus_ftp_client_operationattr_destroy(&(oattr[0]));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not destroy operation attr\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }
   
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
    globus_ftp_client_handleattr_t hattr;
    char gftp_path[1024];

    /* right now, use ftp */
    sprintf(gftp_path, "ftp://127.0.0.1:2811%s", full_path);

    /* init the handle attr */
    ret = globus_ftp_client_handleattr_init(&hattr);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not init handle\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* init the operation. @TODO fix the handle */
    ret = globus_ftp_client_operationattr_init(&(oattr[0]));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not init op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* enable gridftp connection caching */
    ret = globus_ftp_client_handleattr_set_cache_all(&hattr, GLOBUS_TRUE);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not cache connection\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* set the authorization params */
    ret = globus_ftp_client_operationattr_set_authorization(&(oattr[0]), GSS_C_NO_CREDENTIAL, "copej", "", "copej", "");
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not set authorization\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* handle init */
    ret = globus_ftp_client_handle_init(&(gridftp_fh[0]), &hattr);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR reinint handle, %s\n", __func__, globus_object_printable_to_string(globus_error_get(ret)));
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* setup gridftp create */
    globus_mutex_init(&lock, GLOBUS_NULL);
    globus_cond_init(&cond, GLOBUS_NULL);
    file_created=GLOBUS_FALSE;
    create_done=GLOBUS_FALSE;
    create_wb_done=GLOBUS_FALSE;

    /* register put  and write a 0 byte file */
    ret = globus_ftp_client_put(&(gridftp_fh[0]), gftp_path, &(oattr[0]), GLOBUS_NULL, create_cb, GLOBUS_NULL);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register put\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }
    
    /* issue a write */
    globus_byte_t buf= (globus_byte_t)'\0';
    ret = globus_ftp_client_register_write(&(gridftp_fh[0]), (globus_byte_t *)&buf, 0, (globus_off_t)0, GLOBUS_TRUE, create_data_cb, GLOBUS_NULL);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not register write op\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* wait for the callback to finish */
    globus_mutex_lock(&lock);
    while (create_done != GLOBUS_TRUE && create_wb_done != GLOBUS_TRUE)
        globus_cond_wait(&cond, &lock);
    globus_mutex_unlock(&lock);

    /* cleanup handle attr info */
    ret = globus_ftp_client_handleattr_destroy(&hattr);
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not destroy handle attr\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* cleanup op attr info */
    ret = globus_ftp_client_operationattr_destroy(&(oattr[0]));
    if(ret != GLOBUS_SUCCESS)
    {
        fprintf(stderr, "%s : ERROR could not destroy operation attr\n", __func__);
        fprintf(stderr, "%s : exit\n", __func__);
        return ZFSERR_OTHER;
    }

    /* check if found the handle or not */
    if(file_created == GLOBUS_TRUE)
    {
        *created = 1;
    }
    else
    {
        *created = 0;
    }

    return ZFS_OK;
}

static int zoidfs_gridftp_remove(const zoidfs_handle_t * UNUSED(parent_handle),
                      const char * UNUSED(component_name),
                      const char * UNUSED(full_path),
                      zoidfs_cache_hint_t * UNUSED(parent_hint),
                      zoidfs_op_hint_t * UNUSED(op_hint))
{
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
    const zoidfs_handle_t * UNUSED(parent_handle),
    const char * UNUSED(component_name),
    const char * UNUSED(full_path),
    const zoidfs_sattr_t * UNUSED(attr),
    zoidfs_cache_hint_t * UNUSED(parent_hint),
    zoidfs_op_hint_t * UNUSED(op_hint))
{
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
