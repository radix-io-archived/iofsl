#include "zoidfs/zoidfs.h"

/* incomplete impl of IOFWDClient class */
struct IOFWDClient;

/* C wrapper functions for IOFWDClient methods */

void * IOFWDClient_cwrapper_allocate(char * address,
        char * configfile);

void IOFWDClient_cwrapper_free(void * ptr);

int IOFWDClient_cwrapper_init(struct IOFWDClient *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_finalize(struct IOFWDClient *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_create(struct IOFWDClient *,
        const zoidfs_handle_t *, const char *,
        const char *,
        const zoidfs_sattr_t *,
        zoidfs_handle_t *, int *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_remove(struct IOFWDClient *,
        const zoidfs_handle_t *,
        const char *,
        const char *,
        zoidfs_cache_hint_t *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_lookup(struct IOFWDClient *,
        const zoidfs_handle_t *,
        const char *,
        const char *,
        zoidfs_handle_t *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_setattr(struct IOFWDClient *,
        const zoidfs_handle_t *,
        const zoidfs_sattr_t *,
        zoidfs_attr_t *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_getattr(struct IOFWDClient *,
        const zoidfs_handle_t *,
        zoidfs_attr_t *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_null(struct IOFWDClient *,
        zoidfs_op_hint_t *);

int IOFWDClient_cwrapper_read(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * handle,
        size_t mem_count,
        void * mem_starts[],
        const size_t mem_sizes[],
        size_t file_count,
        const zoidfs_file_ofs_t file_starts[],
        zoidfs_file_size_t file_sizes[],
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_commit(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * handle,
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_rename(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * from_parent_handle,
        const char * from_component_name,
        const char * from_full_path,
        const zoidfs_handle_t * to_parent_handle,
        const char * to_component_name,
        const char * to_full_path,
        zoidfs_cache_hint_t * from_parent_hint,
        zoidfs_cache_hint_t * to_parent_hint,
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_link(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * from_parent_handle,
        const char * from_component_name,
        const char * from_full_path,
        const zoidfs_handle_t * to_parent_handle,
        const char * to_component_name,
        const char * to_full_path,
        zoidfs_cache_hint_t * from_parent_hint,
        zoidfs_cache_hint_t * to_parent_hint,
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_symlink(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * from_parent_handle,
        const char * from_component_name,
        const char * from_full_path,
        const zoidfs_handle_t * to_parent_handle,
        const char * to_component_name,
        const char * to_full_path,
        const zoidfs_sattr_t * attr,
        zoidfs_cache_hint_t * from_parent_hint,
        zoidfs_cache_hint_t * to_parent_hint,
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_mkdir(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * parent_handle,
        const char * component_name,
        const char * full_path,
        const zoidfs_sattr_t * attr,
        zoidfs_cache_hint_t * parent_hint,
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_readdir(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * parent_handle,
        zoidfs_dirent_cookie_t cookie,
        size_t * entry_count,
        zoidfs_dirent_t * entries,
        uint32_t flags,
        zoidfs_cache_hint_t * parent_hint,
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_resize(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * handle,
        uint64_t size,
        zoidfs_op_hint_t * op_hint );

int IOFWDClient_cwrapper_readlink(struct IOFWDClient * iofwdclient_ptr,
        const zoidfs_handle_t * handle,
        char * buffer,
        size_t buffer_length,
        zoidfs_op_hint_t * op_hint);

/* clients pointer to the IOFWDClient object */
static struct IOFWDClient * iofwdclient_ptr = NULL;

/* zoidfs API that invokes the C wrappers (which invoke the IOFWCClient object
 * methods */

int zoidfs_init(void)
{
    char * address = NULL;
    char * configfile = NULL;

    if(iofwdclient_ptr == NULL)
        iofwdclient_ptr = IOFWDClient_cwrapper_allocate(address, configfile);

    return IOFWDClient_cwrapper_init(iofwdclient_ptr, NULL);
}

int zoidfs_finalize(void)
{
    int ret = IOFWDClient_cwrapper_finalize(iofwdclient_ptr, NULL);

    if(iofwdclient_ptr)
        IOFWDClient_cwrapper_free(iofwdclient_ptr);

    return ret;
}

int zoidfs_create(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  const zoidfs_sattr_t * attr,
                  zoidfs_handle_t * handle,
                  int * created,
                  zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_create(iofwdclient_ptr, parent_handle,
            component_name, full_path, attr, handle, created, op_hint);
}

int zoidfs_remove(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_cache_hint_t * parent_hint, 
                  zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_remove(iofwdclient_ptr, parent_handle,
            component_name, full_path, parent_hint, op_hint);
}

int zoidfs_lookup(const zoidfs_handle_t * parent_handle,
                  const char * component_name,
                  const char * full_path,
                  zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_lookup(iofwdclient_ptr, parent_handle,
            component_name, full_path, handle, op_hint);
}

int zoidfs_getattr(const zoidfs_handle_t * handle,
                   zoidfs_attr_t * attr,
                   zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_getattr(iofwdclient_ptr, handle, attr, op_hint);
}

int zoidfs_setattr(const zoidfs_handle_t * handle,
                   const zoidfs_sattr_t * sattr,
                   zoidfs_attr_t * attr,
                   zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_setattr(iofwdclient_ptr, handle, sattr,
            attr, op_hint);
}

int zoidfs_null(void)
{
    return IOFWDClient_cwrapper_null(iofwdclient_ptr, NULL);
}

int zoidfs_readlink(const zoidfs_handle_t * handle,
                    char * buffer,
                    size_t buffer_length,
                    zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_readlink(iofwdclient_ptr, handle, buffer,
            buffer_length, op_hint);
}

int zoidfs_read(const zoidfs_handle_t * handle,
                size_t mem_count,
                void * mem_starts[],
                const size_t mem_sizes[],
                size_t file_count,
                const zoidfs_file_ofs_t file_starts[],
                zoidfs_file_size_t file_sizes[],
                zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_read(iofwdclient_ptr, handle, mem_count,
            mem_starts, mem_sizes, file_count, file_starts, file_sizes, op_hint);
}

int zoidfs_commit(const zoidfs_handle_t * handle,
                  zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_commit(iofwdclient_ptr, handle, op_hint);
}

int zoidfs_rename(const zoidfs_handle_t * from_parent_handle,
                  const char * from_component_name,
                  const char * from_full_path,
                  const zoidfs_handle_t * to_parent_handle,
                  const char * to_component_name,
                  const char * to_full_path,
                  zoidfs_cache_hint_t * from_parent_hint,
                  zoidfs_cache_hint_t * to_parent_hint,
                  zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_rename(iofwdclient_ptr, from_parent_handle,
            from_component_name, from_full_path, to_parent_handle,
            to_component_name, to_full_path, from_parent_hint, to_parent_hint,
            op_hint);
}

int zoidfs_link(const zoidfs_handle_t * from_parent_handle,
                const char * from_component_name,
                const char * from_full_path,
                const zoidfs_handle_t * to_parent_handle,
                const char * to_component_name,
                const char * to_full_path,
                zoidfs_cache_hint_t * from_parent_hint,
                zoidfs_cache_hint_t * to_parent_hint,
                zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_link(iofwdclient_ptr, from_parent_handle,
            from_component_name, from_full_path, to_parent_handle,
            to_component_name, to_full_path, from_parent_hint, to_parent_hint,
            op_hint);
}

int zoidfs_symlink(const zoidfs_handle_t * from_parent_handle,
                   const char * from_component_name,
                   const char * from_full_path,
                   const zoidfs_handle_t * to_parent_handle,
                   const char * to_component_name,
                   const char * to_full_path,
                   const zoidfs_sattr_t * attr,
                   zoidfs_cache_hint_t * from_parent_hint,
                   zoidfs_cache_hint_t * to_parent_hint,
                   zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_symlink(iofwdclient_ptr, from_parent_handle,
            from_component_name, from_full_path, to_parent_handle,
            to_component_name, to_full_path, attr, from_parent_hint,
            to_parent_hint, op_hint);
}

int zoidfs_mkdir(const zoidfs_handle_t * parent_handle,
                 const char * component_name,
                 const char * full_path,
                 const zoidfs_sattr_t * attr,
                 zoidfs_cache_hint_t * parent_hint,
                 zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_mkdir(iofwdclient_ptr, parent_handle,
            component_name, full_path, attr, parent_hint, op_hint);
}

int zoidfs_readdir(const zoidfs_handle_t * parent_handle,
                   zoidfs_dirent_cookie_t cookie,
                   size_t * entry_count,
                   zoidfs_dirent_t * entries,
                   uint32_t flags,
                   zoidfs_cache_hint_t * parent_hint,
                   zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_readdir(iofwdclient_ptr, parent_handle, cookie,
            entry_count, entries, flags, parent_hint, op_hint);
}

int zoidfs_resize(const zoidfs_handle_t * handle,
                  uint64_t size,
                  zoidfs_op_hint_t * op_hint )
{
    return IOFWDClient_cwrapper_resize(iofwdclient_ptr, handle, size, op_hint);
}
