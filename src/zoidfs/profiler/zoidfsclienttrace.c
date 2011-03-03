/*
 * zoidfsclienttrace.c
 * Simple ZoidFS trace library. Reference for future instrumentation libs.
 *
 * Jason Cope <copej@mcs.anl.gov>
 *
 */

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"

#include <stdio.h>
#include <time.h>
#include <pthread.h>

/*
 * debug and trace tools
 */
#define ZOIDFS_PRINT_COUNTER(__counter)    fprintf(stderr, "\t%s = %u\n", #__counter, __counter##_call_count)
#define ZOIDFS_PRINT_TIME(__timer)    fprintf(stderr, "\t%s = %f\n", #__timer, __timer##_time)

/* is tracing enabled */
#if ZOIDFS_TRACING_ENABLED
#define ZOIDFS_TRACE() fprintf(stderr, "ZOIDFS API TRACE: %s()\n", __func__)
#else
#define ZOIDFS_TRACE() /* no tracing */
#endif

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

static double zoidfs_null_time = 0.0;
static double zoidfs_init_time = 0.0;
static double zoidfs_finalize_time = 0.0;
static double zoidfs_getattr_time = 0.0;
static double zoidfs_setattr_time = 0.0;
static double zoidfs_create_time = 0.0;
static double zoidfs_commit_time = 0.0;
static double zoidfs_read_time = 0.0;
static double zoidfs_write_time = 0.0;
static double zoidfs_link_time = 0.0;
static double zoidfs_lookup_time = 0.0;
static double zoidfs_symlink_time = 0.0;
static double zoidfs_mkdir_time = 0.0;
static double zoidfs_readdir_time = 0.0;
static double zoidfs_remove_time = 0.0;
static double zoidfs_rename_time = 0.0;
static double zoidfs_resize_time = 0.0;
static double zoidfs_readlink_time = 0.0;

static pthread_mutex_t zoidfs_null_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_init_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_finalize_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_getattr_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_setattr_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_create_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_commit_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_read_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_write_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_link_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_lookup_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_symlink_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_mkdir_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_readdir_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_remove_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_rename_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_resize_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t zoidfs_readlink_mutex = PTHREAD_MUTEX_INITIALIZER;

int zoidfs_prof_get_time(struct timespec * timeval)
{
    clock_gettime( CLOCK_REALTIME, timeval );
    return 0;
}

double zoidfs_prof_elapsed_time(struct timespec * t1, struct timespec * t2)
{
    return ((double) (t2->tv_sec - t1->tv_sec) +
        1.0e-9 * (double) (t2->tv_nsec - t1->tv_nsec) );
}

#ifndef ZOIDFS_DISPATCHER_PROF
#define ZOIDFS_PROF_TIME(__func, __tval, __mutex)   \
do{                                                 \
    struct timespec t1, t2;                         \
    zoidfs_prof_get_time(&t1);                      \
    __func;                                         \
    zoidfs_prof_get_time(&t2);                      \
    pthread_mutex_lock(&__mutex);                   \
    __tval += zoidfs_prof_elapsed_time(&t1, &t2);   \
    pthread_mutex_unlock(&__mutex);                 \
}while(0)

#define ZOIDFS_PROF_INC_COUNTER(__counter, __mutex) \
do{                                                 \
    pthread_mutex_lock(&__mutex);                   \
    __counter++;                                    \
    pthread_mutex_unlock(&__mutex);                 \
}while(0)
#else
#define ZOIDFS_PROF_TIME(__func, __tval, __mutex)   \
do{                                                 \
    __func;                                         \
}while(0)

#define ZOIDFS_PROF_INC_COUNTER(__counter, __mutex) \
do{                                                 \
}while(0)
#endif
/*
 * zoidfs_null
 * This function implements a noop operation. The IOD returns a 1-byte message
 * to the CN.
 *
 */
int zoidfs_null(void) {

    int ret = 0;
    
    ZOIDFS_PROF_INC_COUNTER(zoidfs_null_call_count, zoidfs_null_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_null(ZOIDFS_NO_OP_HINT), zoidfs_null_time, zoidfs_null_mutex);

    return ret;
}


/*
 * zoidfs_getattr
 * This function retrieves the attributes associated with the file handle from
 * the ION.
 */
int zoidfs_getattr(const zoidfs_handle_t *handle, zoidfs_attr_t *attr, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    ZOIDFS_PROF_INC_COUNTER(zoidfs_getattr_call_count, zoidfs_getattr_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_getattr(handle, attr, op_hint), zoidfs_getattr_time, zoidfs_getattr_mutex);

    return ret;
}


/*
 * zoidfs_setattr
 * This function sets the attributes associated with the file handle.
 */
int zoidfs_setattr(const zoidfs_handle_t *handle, const zoidfs_sattr_t *sattr,
                   zoidfs_attr_t *attr, zoidfs_op_hint_t * op_hint) {

    int ret = 0;

    ZOIDFS_PROF_INC_COUNTER(zoidfs_setattr_call_count, zoidfs_setattr_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_setattr(handle, sattr, attr, op_hint), zoidfs_setattr_time, zoidfs_setattr_mutex);

    return ret;
}


/*
 * zoidfs_readlink
 * This function reads a symbolic link.
 */
int zoidfs_readlink(const zoidfs_handle_t *handle, char *buffer,
                    size_t buffer_length, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    ZOIDFS_PROF_INC_COUNTER(zoidfs_readlink_call_count, zoidfs_readlink_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_readlink(handle, buffer, buffer_length, op_hint), zoidfs_readlink_time, zoidfs_readlink_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_lookup_call_count, zoidfs_lookup_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_lookup(parent_handle, component_name, full_path, handle, op_hint), zoidfs_lookup_time, zoidfs_lookup_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_remove_call_count, zoidfs_remove_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_remove(parent_handle, component_name, full_path, parent_hint, op_hint), zoidfs_remove_time, zoidfs_remove_mutex);

    return ret;
}


/*
 * zoidfs_commit
 * This function flushes the buffers associated with the file handle.
 */
int zoidfs_commit(const zoidfs_handle_t *handle, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    ZOIDFS_PROF_INC_COUNTER(zoidfs_commit_call_count, zoidfs_commit_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_commit(handle, op_hint), zoidfs_commit_time, zoidfs_commit_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_create_call_count, zoidfs_create_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_create(parent_handle, component_name, full_path, sattr, handle, created, op_hint), zoidfs_create_time, zoidfs_create_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_rename_call_count, zoidfs_rename_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_rename(from_parent_handle, from_component_name, from_full_path, to_parent_handle, to_component_name, to_full_path, from_parent_hint, to_parent_hint, op_hint), zoidfs_rename_time, zoidfs_rename_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_link_call_count, zoidfs_link_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_link(from_parent_handle, from_component_name, from_full_path, to_parent_handle, to_component_name, to_full_path, from_parent_hint, to_parent_hint, op_hint), zoidfs_link_time, zoidfs_link_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_symlink_call_count, zoidfs_symlink_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_symlink(from_parent_handle, from_component_name, from_full_path, to_parent_handle, to_component_name, to_full_path, sattr, from_parent_hint, to_parent_hint, op_hint), zoidfs_symlink_time, zoidfs_symlink_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_mkdir_call_count, zoidfs_mkdir_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_mkdir(parent_handle, component_name, full_path, sattr, parent_hint, op_hint), zoidfs_mkdir_time, zoidfs_mkdir_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_readdir_call_count, zoidfs_readdir_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_readdir(parent_handle, cookie, entry_count_, entries, flags, parent_hint, op_hint), zoidfs_readdir_time, zoidfs_readdir_mutex);

    return ret;
}


/*
 * zoidfs_resize
 * This function resizes the file associated with the file handle.
 */
int zoidfs_resize(const zoidfs_handle_t *handle, uint64_t size, zoidfs_op_hint_t * op_hint) {
    int ret = 0;

    ZOIDFS_PROF_INC_COUNTER(zoidfs_resize_call_count, zoidfs_resize_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_resize(handle, size, op_hint), zoidfs_resize_time, zoidfs_resize_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_write_call_count, zoidfs_write_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_write(handle, mem_count_, mem_starts, mem_sizes_, file_count_, file_starts, file_sizes, op_hint), zoidfs_write_time, zoidfs_write_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_read_call_count, zoidfs_read_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_read(handle, mem_count_, mem_starts, mem_sizes_, file_count_, file_starts, file_sizes, op_hint), zoidfs_read_time, zoidfs_read_mutex);

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

    ZOIDFS_PROF_INC_COUNTER(zoidfs_init_call_count, zoidfs_init_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_init(ZOIDFS_NO_OP_HINT), zoidfs_init_time, zoidfs_init_mutex);

    return ret;
}

/*
 * zoidfs_finalize
 * Finalize the client subsystems.
 */
int zoidfs_finalize(void) {
    int ret = 0;

    ZOIDFS_PROF_INC_COUNTER(zoidfs_finalize_call_count, zoidfs_finalize_mutex);
    ZOIDFS_TRACE();

    ZOIDFS_PROF_TIME(ret = Pzoidfs_finalize(ZOIDFS_NO_OP_HINT), zoidfs_finalize_time, zoidfs_finalize_mutex);

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

    fprintf(stderr, "ZOIDFS API CALL TIMES:\n");
    ZOIDFS_PRINT_TIME(zoidfs_null);
    ZOIDFS_PRINT_TIME(zoidfs_init);
    ZOIDFS_PRINT_TIME(zoidfs_finalize);
    ZOIDFS_PRINT_TIME(zoidfs_getattr);
    ZOIDFS_PRINT_TIME(zoidfs_setattr);
    ZOIDFS_PRINT_TIME(zoidfs_create);
    ZOIDFS_PRINT_TIME(zoidfs_commit);
    ZOIDFS_PRINT_TIME(zoidfs_read);
    ZOIDFS_PRINT_TIME(zoidfs_write);
    ZOIDFS_PRINT_TIME(zoidfs_link);
    ZOIDFS_PRINT_TIME(zoidfs_lookup);
    ZOIDFS_PRINT_TIME(zoidfs_symlink);
    ZOIDFS_PRINT_TIME(zoidfs_mkdir);
    ZOIDFS_PRINT_TIME(zoidfs_readdir);
    ZOIDFS_PRINT_TIME(zoidfs_remove);
    ZOIDFS_PRINT_TIME(zoidfs_rename);
    ZOIDFS_PRINT_TIME(zoidfs_resize);
    ZOIDFS_PRINT_TIME(zoidfs_readlink);

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
