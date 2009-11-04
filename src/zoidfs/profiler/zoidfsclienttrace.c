/*
 * zoidfsclienttrace.c
 * Simple ZoidFS trace library. Reference for future instrumentation libs.
 *
 * Jason Cope <copej@mcs.anl.gov>
 *    
 * @TODO: fix explicit uint64_t -> zoidfs_file_ofs/size_t
 */

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

#include <stdio.h>

/*
 * debug and trace tools
 */
#define ZOIDFS_PRINT_COUNTER(__counter)    fprintf(stderr, "\t%s = %u\n", #__counter, __counter##_call_count)
#define ZOIDFS_TRACE() fprintf(stderr, "ZOIDFS API TRACE: %s()\n", __func__) 

/*
 * call counts
 */
static unsigned int zoidfs_null_call_count = 0;
static unsigned int zoidfs_init_call_count = 0;
static unsigned int zoidfs_finalize_call_count = 0;
static unsigned int zoidfs_getattr_call_count = 0;
static unsigned int zoidfs_setattr_call_count = 0;
static unsigned int zoidfs_create_call_count = 0;
static unsigned int zoidfs_commit_call_count = 0;
static unsigned int zoidfs_read_call_count = 0;
static unsigned int zoidfs_write_call_count = 0;
static unsigned int zoidfs_link_call_count = 0;
static unsigned int zoidfs_lookup_call_count = 0;
static unsigned int zoidfs_symlink_call_count = 0;
static unsigned int zoidfs_mkdir_call_count = 0;
static unsigned int zoidfs_readdir_call_count = 0;
static unsigned int zoidfs_remove_call_count = 0;
static unsigned int zoidfs_rename_call_count = 0;
static unsigned int zoidfs_resize_call_count = 0;
static unsigned int zoidfs_readlink_call_count = 0;

/*
 * zoidfs_null
 * This function implements a noop operation. The IOD returns a 1-byte message
 * to the CN.
 *
 */
int zoidfs_null(void) {

    int ret = 0;

    zoidfs_null_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_null();
    
    return ret;
}


/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle from
 * the ION.
 */
int zoidfs_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_getattr_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_getattr(handle, attr, op_hint);

    return ret;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
int zoidfs_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t *attr, zoidfs_op_hint_t * op_hint) {

    int ret = 0;

    zoidfs_setattr_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_setattr(handle, sattr, attr, op_hint);

    return ret;
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 */
int zoidfs_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_readlink_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_readlink(handle, buffer, buffer_length, op_hint);

    return ret;
}


/*
 * zoidfs_lookup
 * This function returns the file handle associated with the given file or
 * directory name.
 */
int zoidfs_lookup(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_handle_t *handle, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_lookup_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_lookup(parent_handle, component_name, full_path, handle, op_hint);

    return ret;
}


/*
 * zoidfs_remove
 * This function removes the given file or directory.
 */
int zoidfs_remove(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  zoidfs_cache_hint_t *parent_hint, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_remove_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_remove(parent_handle, component_name, full_path, parent_hint, op_hint);

    return ret;
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
int zoidfs_commit(const zoidfs_handle_t *handle, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_commit_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_commit(handle, op_hint);

    return ret;
}


/*
 * zoidfs_create
 * This function creates a new file.
 */
int zoidfs_create(const zoidfs_handle_t *parent_handle,
                  const char *component_name, const char *full_path,
                  const zoidfs_sattr_t *sattr, zoidfs_handle_t *handle,
                  int *created, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_create_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_create(parent_handle, component_name, full_path, sattr, handle, created, op_hint);

    return ret;
}

/*
 * zoidfs_rename
 * This function renames an existing file or directory.
 */
int zoidfs_rename(const zoidfs_handle_t *from_parent_handle,
                  const char *from_component_name,
                  const char *from_full_path,
                  const zoidfs_handle_t *to_parent_handle,
                  const char *to_component_name,
                  const char *to_full_path,
                  zoidfs_cache_hint_t *from_parent_hint,
                  zoidfs_cache_hint_t *to_parent_hint, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_rename_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_rename(from_parent_handle, from_component_name, from_full_path, to_parent_handle, to_component_name, to_full_path, from_parent_hint, to_parent_hint, op_hint);

    return ret;
}


/*
 * zoidfs_link
 * This function creates a hard link.
 */
int zoidfs_link(const zoidfs_handle_t *from_parent_handle,
                const char *from_component_name,
                const char *from_full_path,
                const zoidfs_handle_t *to_parent_handle,
                const char *to_component_name,
                const char *to_full_path,
                zoidfs_cache_hint_t *from_parent_hint,
                zoidfs_cache_hint_t *to_parent_hint, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_link_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_link(from_parent_handle, from_component_name, from_full_path, to_parent_handle, to_component_name, to_full_path, from_parent_hint, to_parent_hint, op_hint);

    return ret;
}


/*
 * zoidfs_symlink
 * This function creates a symbolic link.
 */
int zoidfs_symlink(const zoidfs_handle_t *from_parent_handle,
                   const char *from_component_name,
                   const char *from_full_path,
                   const zoidfs_handle_t *to_parent_handle,
                   const char *to_component_name,
                   const char *to_full_path,
                   const zoidfs_sattr_t *sattr,
                   zoidfs_cache_hint_t *from_parent_hint,
                   zoidfs_cache_hint_t *to_parent_hint, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_symlink_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_symlink(from_parent_handle, from_component_name, from_full_path, to_parent_handle, to_component_name, to_full_path, sattr, from_parent_hint, to_parent_hint, op_hint);

    return ret;
}

/*
 * zoidfs_mkdir
 * This function creates a new directory.
 */
int zoidfs_mkdir(const zoidfs_handle_t *parent_handle,
                 const char *component_name, const char *full_path,
                 const zoidfs_sattr_t *sattr,
                 zoidfs_cache_hint_t *parent_hint, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_mkdir_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_mkdir(parent_handle, component_name, full_path, sattr, parent_hint, op_hint);

    return ret;
}


/*
 * zoidfs_readdir
 * This function returns the dirents from the specified parent directory. The
 * cookie is a pointer which specifies where in the directory to start
 * fetching the dirents from.
 */
int zoidfs_readdir(const zoidfs_handle_t *parent_handle,
                   zoidfs_dirent_cookie_t cookie, size_t *entry_count_,
                   zoidfs_dirent_t *entries, uint32_t flags,
                   zoidfs_cache_hint_t *parent_hint, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_readdir_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_readdir(parent_handle, cookie, entry_count_, entries, flags, parent_hint, op_hint);

    return ret;
}


/*
 * zoidfs_resize
 * This function resizes the file associated with the file handle.
 */
int zoidfs_resize(const zoidfs_handle_t *handle, uint64_t size, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_resize_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_resize(handle, size, op_hint);

    return ret;
}

/*
 * zoidfs_write
 * This function implements the zoidfs write call.
 */
int zoidfs_write(const zoidfs_handle_t *handle, size_t mem_count_,
                 const void *mem_starts[], const size_t mem_sizes_[],
                 size_t file_count_, const zoidfs_file_ofs_t file_starts[],
                 zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_write_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_write(handle, mem_count_, mem_starts, mem_sizes_, file_count_, file_starts, file_sizes, op_hint);

    return ret;
}


/*
 * zoidfs_read
 * This function implements the zoidfs read call.
 */
int zoidfs_read(const zoidfs_handle_t *handle, size_t mem_count_,
                void *mem_starts[], const size_t mem_sizes_[],
                size_t file_count_, const zoidfs_file_ofs_t file_starts[],
                zoidfs_file_size_t file_sizes[], zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    zoidfs_read_call_count++;
    ZOIDFS_TRACE();

    ret = zoidfs_read(handle, mem_count_, mem_starts, mem_sizes_, file_count_, file_starts, file_sizes, op_hint);

    return ret;
}

/*
 * zoidfs_init
 * Initialize the client subsystems.
 */
int zoidfs_init(void) {
    int ret = 0;

    /* init the counters to 0 */
    zoidfs_null_call_count = 0;
    zoidfs_init_call_count = 0;
    zoidfs_finalize_call_count = 0;
    zoidfs_getattr_call_count = 0;
    zoidfs_setattr_call_count = 0;
    zoidfs_create_call_count = 0;
    zoidfs_commit_call_count = 0;
    zoidfs_read_call_count = 0;
    zoidfs_write_call_count = 0;
    zoidfs_link_call_count = 0;
    zoidfs_lookup_call_count = 0;
    zoidfs_symlink_call_count = 0;
    zoidfs_mkdir_call_count = 0;
    zoidfs_readdir_call_count = 0;
    zoidfs_remove_call_count = 0;
    zoidfs_rename_call_count = 0;
    zoidfs_resize_call_count = 0;
    zoidfs_readlink_call_count = 0;

    zoidfs_init_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_init();

    return ret;
}

/*
 * zoidfs_finalize
 * Finalize the client subsystems.
 */
int zoidfs_finalize(void) {
    int ret = 0;

    zoidfs_finalize_call_count++;
    ZOIDFS_TRACE();

    ret = Pzoidfs_finalize();

    /* print the counter values */
    fprintf(stderr, "ZOIDFS API CALL COUNTS:\n");
    ZOIDFS_PRINT_COUNTER(zoidfs_null);
    ZOIDFS_PRINT_COUNTER(zoidfs_init);
    ZOIDFS_PRINT_COUNTER(zoidfs_finalize);
    ZOIDFS_PRINT_COUNTER(zoidfs_getattr);
    ZOIDFS_PRINT_COUNTER(zoidfs_setattr);
    ZOIDFS_PRINT_COUNTER(zoidfs_create);
    ZOIDFS_PRINT_COUNTER(zoidfs_commit);
    ZOIDFS_PRINT_COUNTER(zoidfs_read);
    ZOIDFS_PRINT_COUNTER(zoidfs_write);
    ZOIDFS_PRINT_COUNTER(zoidfs_link);
    ZOIDFS_PRINT_COUNTER(zoidfs_lookup);
    ZOIDFS_PRINT_COUNTER(zoidfs_symlink);
    ZOIDFS_PRINT_COUNTER(zoidfs_mkdir);
    ZOIDFS_PRINT_COUNTER(zoidfs_readdir);
    ZOIDFS_PRINT_COUNTER(zoidfs_remove);
    ZOIDFS_PRINT_COUNTER(zoidfs_rename);
    ZOIDFS_PRINT_COUNTER(zoidfs_resize);
    ZOIDFS_PRINT_COUNTER(zoidfs_readlink);

    return ret;
}



/*
 * Local variables:
 *  mode: c
 *  c-indent-level: 4
 *  c-basic-offset: 4
 * End:
 *
 * vim: ft=c ts=4 sts=4 sw=4 expandtab
 */
