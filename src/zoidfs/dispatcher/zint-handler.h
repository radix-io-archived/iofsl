#ifndef ZINT_HANDLER_H
#define ZINT_HANDLER_H

#include "zoidfs/zoidfs.h"

typedef int zint_handle_type_t;

static inline uint32_t ZOIDFS_HANDLE_TYPE (const zoidfs_handle_t * h)
{
   return * (uint32_t*) (&h->data[0]);
}

static inline void ZINT_MARK_HANDLE (const zoidfs_handle_t * h, uint32_t type)
{
   *((uint32_t*) (&h->data[0])) = type;
}

static inline void ZINT_COPY_CLEANUP_HANDLE (const zoidfs_handle_t * source,
      zoidfs_handle_t * dest)
{
   *dest = *source;
   ZINT_MARK_HANDLE(dest, 0);
}

static inline void ZINT_CLEANUP_HANDLE (zoidfs_handle_t * handle)
{
   ZINT_MARK_HANDLE (handle, 0);
}

/* function to compare two handles, but ignoring the area reserverd for the
 * dispatcher return 1 if equal, 0 if noneq */
int zoidfs_dispatch_handle_eq (const zoidfs_handle_t * h1, const
      zoidfs_handle_t * h2);


typedef int (* zint_null_handler_t)(void);

typedef int (* zint_getattr_handler_t)(const zoidfs_handle_t * handle,
                                       zoidfs_attr_t * attr);

typedef int (* zint_setattr_handler_t)(const zoidfs_handle_t * handle,
                                       const zoidfs_sattr_t * sattr,
                                       zoidfs_attr_t * attr);

typedef int (* zint_lookup_handler_t)(const zoidfs_handle_t * parent_handle,
                                      const char * component_name,
                                      const char * full_path,
                                      zoidfs_handle_t * handle);

typedef int (* zint_readlink_handler_t)(const zoidfs_handle_t * handle,
                                        char * buffer,
                                        size_t buffer_length);

typedef int (* zint_read_handler_t)(const zoidfs_handle_t * handle,
                                    size_t mem_count,
                                    void * mem_starts[],
                                    const size_t mem_sizes[],
                                    size_t file_count,
                                    const uint64_t file_starts[],
                                    uint64_t file_sizes[]);

typedef int (* zint_write_handler_t)(const zoidfs_handle_t * handle,
                                     size_t mem_count,
                                     const void * mem_starts[],
                                     const size_t mem_sizes[],
                                     size_t file_count,
                                     const uint64_t file_starts[],
                                     uint64_t file_sizes[]);

typedef int (* zint_commit_handler_t)(const zoidfs_handle_t * handle);

typedef int (* zint_create_handler_t)(const zoidfs_handle_t * parent_handle,
                                      const char * component_name,
                                      const char * full_path,
                                      const zoidfs_sattr_t * attr,
                                      zoidfs_handle_t * handle,
                                      int * created);

typedef int (* zint_remove_handler_t)(const zoidfs_handle_t * parent_handle,
                                      const char * component_name,
                                      const char * full_path,
                                      zoidfs_cache_hint_t * parent_hint);

typedef int (* zint_rename_handler_t)(
    const zoidfs_handle_t * from_parent_handle,
    const char * from_component_name,
    const char * from_full_path,
    const zoidfs_handle_t * to_parent_handle,
    const char * to_component_name,
    const char * to_full_path,
    zoidfs_cache_hint_t * from_parent_hint,
    zoidfs_cache_hint_t * to_parent_hint);

typedef int (* zint_link_handler_t)(const zoidfs_handle_t * from_parent_handle,
                                    const char * from_component_name,
                                    const char * from_full_path,
                                    const zoidfs_handle_t * to_parent_handle,
                                    const char * to_component_name,
                                    const char * to_full_path,
                                    zoidfs_cache_hint_t * from_parent_hint,
                                    zoidfs_cache_hint_t * to_parent_hint);

typedef int (* zint_symlink_handler_t)(
    const zoidfs_handle_t * from_parent_handle,
    const char * from_component_name,
    const char * from_full_path,
    const zoidfs_handle_t * to_parent_handle,
    const char * to_component_name,
    const char * to_full_path,
    const zoidfs_sattr_t * attr,
    zoidfs_cache_hint_t * from_parent_hint,
    zoidfs_cache_hint_t * to_parent_hint);

typedef int (* zint_mkdir_handler_t)(const zoidfs_handle_t * parent_handle,
                                     const char * component_name,
                                     const char * full_path,
                                     const zoidfs_sattr_t * attr,
                                     zoidfs_cache_hint_t * parent_hint);

typedef int (* zint_readdir_handler_t)(const zoidfs_handle_t * parent_handle,
                                       zoidfs_dirent_cookie_t cookie,
                                       size_t * entry_count,
                                       zoidfs_dirent_t * entries,
                                       uint32_t flags,
                                       zoidfs_cache_hint_t * parent_hint);

typedef int (* zint_resize_handler_t)(const zoidfs_handle_t * handle,
                                      uint64_t size);

typedef int (* zint_resolve_path_handler_t)(const char * local_path,
                                            char * fs_path,
                                            int fs_path_max,
                                            zoidfs_handle_t * newhandle,
                                            int * usenew);

typedef int (* zint_init_t)(void);

typedef int (* zint_finalize_t)(void);

typedef int (*zint_settype_handler_t) (int);

typedef struct
{
    zint_null_handler_t null;
    zint_getattr_handler_t getattr;
    zint_setattr_handler_t setattr;
    zint_lookup_handler_t lookup;
    zint_readlink_handler_t readlink;
    zint_read_handler_t read;
    zint_write_handler_t write;
    zint_commit_handler_t commit;
    zint_create_handler_t create;
    zint_remove_handler_t remove;
    zint_rename_handler_t rename;
    zint_link_handler_t link;
    zint_symlink_handler_t symlink;
    zint_mkdir_handler_t mkdir;
    zint_readdir_handler_t readdir;
    zint_resize_handler_t resize;
    zint_resolve_path_handler_t resolve_path;
    zint_init_t init;
    zint_finalize_t finalize;
} zint_handler_t;


int zint_locate_handler_handle (const zoidfs_handle_t * handle,
        zint_handler_t ** handler);

int zint_locate_handler_path (const char * path,
                                      zint_handler_t ** handler,
                                      char * newpath,
                                      int newpath_maxlen,
                                      zoidfs_handle_t * newhandle,
                                      int * usenew);

zint_handler_t * zint_get_handler(zint_handle_type_t type);

zint_handler_t * zint_get_handler_from_path(
    const char * path,
    char * newpath,
    int newpath_maxlen, int *id, zoidfs_handle_t * newhandle,
    int * usenew);


int zint_ping_handlers ();

int zint_initialize_handlers ();

int zint_finalize_handlers ();

#endif /* ZINT_HANDLER_H */

/*
 * Local variables:
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ts=8 sts=4 sw=4 expandtab
 */
