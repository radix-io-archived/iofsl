#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>

#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

#define NAMESIZE 255
#define BUFSIZE (8 * 1024 * 1024)

#include "CUnit/Basic.h"

/* path variables */
static char link_component_filename[NAMESIZE], link_component_dirname[NAMESIZE];
static char link_fullpath_filename[NAMESIZE], link_fullpath_dirname[NAMESIZE];
static char symlink_component_filename[NAMESIZE], symlink_component_dirname[NAMESIZE];
static char symlink_fullpath_filename[NAMESIZE], symlink_fullpath_dirname[NAMESIZE];
static char fullpath_dirname[NAMESIZE], component_dirname[NAMESIZE];
static char fullpath_filename[NAMESIZE], component_filename[NAMESIZE];
static char rename_fullpath_dirname[NAMESIZE], rename_component_dirname[NAMESIZE];
static char rename_fullpath_filename[NAMESIZE], rename_component_filename[NAMESIZE];

/* basedir handle */
static zoidfs_handle_t basedir_handle;

/* setup the file paths */
int init_path_names(char * mpt)
{
    snprintf(symlink_component_filename, NAMESIZE, "symlink_comp_file");
    snprintf(symlink_component_dirname, NAMESIZE, "symlink_comp_dir");
    snprintf(symlink_fullpath_filename, NAMESIZE, "%s/symlink_full_file", mpt);
    snprintf(symlink_fullpath_dirname, NAMESIZE, "%s/symlink_full_dir", mpt);

    snprintf(link_component_filename, NAMESIZE, "link_comp_file");
    snprintf(link_component_dirname, NAMESIZE, "link_comp_dir");
    snprintf(link_fullpath_filename, NAMESIZE, "%s/link_full_file", mpt);
    snprintf(link_fullpath_dirname, NAMESIZE, "%s/link_full_dir", mpt);

    snprintf(component_filename, NAMESIZE, "test-zoidfs-file-comp");
    snprintf(component_dirname, NAMESIZE, "test-zoidfs-dir-comp");
    snprintf(fullpath_filename, NAMESIZE, "%s/test-zoidfs-file-full", mpt);
    snprintf(fullpath_dirname, NAMESIZE, "%s/test-zoidfs-dir-full", mpt);

    snprintf(rename_component_filename, NAMESIZE, "test-zoidfs-file-comp-rename");
    snprintf(rename_component_dirname, NAMESIZE, "test-zoidfs-dir-comp-rename");
    snprintf(rename_fullpath_filename, NAMESIZE, "%s/test-zoidfs-file-full-rename", mpt);
    snprintf(rename_fullpath_dirname, NAMESIZE, "%s/test-zoidfs-dir-full-rename", mpt);

    return 0;
}

int init_basedir_handle(char * mpt)
{
    return zoidfs_lookup(NULL, NULL, mpt, &basedir_handle, ZOIDFS_NO_OP_HINT);
}

int testCREATE(void)
{
    int created = 0;
    struct timeval now;
    zoidfs_sattr_t sattr;
    zoidfs_handle_t fhandle;

    /* set the attrs */
    sattr.mask = ZOIDFS_ATTR_SETABLE;
    sattr.mode = 0755;
    sattr.uid = getuid();
    sattr.gid = getgid();

    gettimeofday(&now, NULL);
    sattr.atime.seconds = now.tv_sec;
    sattr.atime.nseconds = now.tv_usec;
    sattr.mtime.seconds = now.tv_sec;
    sattr.mtime.nseconds = now.tv_usec;

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_create(&basedir_handle, component_filename, NULL, &sattr, &fhandle, &created, ZOIDFS_NO_OP_HINT));

    /* create a file using the base handle and component name*/
    CU_ASSERT(ZFS_OK == zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &fhandle, &created, ZOIDFS_NO_OP_HINT));

    return 0;
}

int testLOOKUP(void)
{
    zoidfs_handle_t fhandle;

    CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, component_dirname, NULL, &fhandle, ZOIDFS_NO_OP_HINT));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, fullpath_filename, &fhandle, ZOIDFS_NO_OP_HINT));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, fullpath_dirname, &fhandle, ZOIDFS_NO_OP_HINT));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, symlink_component_filename, NULL, &fhandle, ZOIDFS_NO_OP_HINT));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(&basedir_handle, symlink_component_dirname, NULL, &fhandle, ZOIDFS_NO_OP_HINT));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, symlink_fullpath_filename, &fhandle, ZOIDFS_NO_OP_HINT));
    CU_ASSERT(ZFS_OK == zoidfs_lookup(NULL, NULL, symlink_fullpath_dirname, &fhandle, ZOIDFS_NO_OP_HINT));

    return 0;
}

/*
 * write macro
 */
#define ZOIDFS_WRITE_COMP_DEF(_N, _BSIZE) \
int _do_zoidfs_write_comp_nbuf##_N##_bsize##_BSIZE() \
{\
    int ret = 0; \
    size_t mem_sizes[_N]; \
    size_t _foff = 0; \
    size_t mem_count, file_count; \
    uint64_t file_sizes[_N], file_starts[_N]; \
    void *mem_starts_write[_N]; \
    size_t _i = 0; \
    zoidfs_handle_t handle; \
    int created = 0; \
    struct timeval now; \
    zoidfs_sattr_t sattr; \
    sattr.mask = ZOIDFS_ATTR_SETABLE; \
    sattr.mode = 0755; \
    sattr.uid = getuid(); \
    sattr.gid = getgid(); \
    gettimeofday(&now, NULL); \
    sattr.atime.seconds = now.tv_sec; \
    sattr.atime.nseconds = now.tv_usec; \
    sattr.mtime.seconds = now.tv_sec; \
    sattr.mtime.nseconds = now.tv_usec; \
    mem_count = _N; \
    file_count = _N; \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        mem_starts_write[_i] = malloc(_BSIZE); \
        memset(mem_starts_write[_i], 'a', _BSIZE); \
        file_sizes[_i] = mem_sizes[_i] = _BSIZE; \
        file_starts[_i] = _foff; \
        _foff += _BSIZE; \
    } \
    zoidfs_create(&basedir_handle, component_filename, NULL, &sattr, &handle, &created, ZOIDFS_NO_OP_HINT); \
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &handle, ZOIDFS_NO_OP_HINT);\
    ret = zoidfs_write(&handle, mem_count, (const void **)mem_starts_write, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); \
    zoidfs_remove(&basedir_handle, component_filename, NULL, NULL, ZOIDFS_NO_OP_HINT); \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        free(mem_starts_write[_i]); \
    } \
    return ret; \
}

#define ZOIDFS_WRITE_FULL_DEF(_N, _BSIZE) \
int _do_zoidfs_write_full_nbuf##_N##_bsize##_BSIZE() \
{\
    int ret = 0; \
    size_t mem_sizes[_N]; \
    size_t _foff = 0; \
    size_t mem_count, file_count; \
    uint64_t file_sizes[_N], file_starts[_N]; \
    void *mem_starts_write[_N]; \
    size_t _i = 0; \
    zoidfs_handle_t handle; \
    int created = 0; \
    struct timeval now; \
    zoidfs_sattr_t sattr; \
    sattr.mask = ZOIDFS_ATTR_SETABLE; \
    sattr.mode = 0755; \
    sattr.uid = getuid(); \
    sattr.gid = getgid(); \
    gettimeofday(&now, NULL); \
    sattr.atime.seconds = now.tv_sec; \
    sattr.atime.nseconds = now.tv_usec; \
    sattr.mtime.seconds = now.tv_sec; \
    sattr.mtime.nseconds = now.tv_usec; \
    mem_count = _N; \
    file_count = _N; \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        mem_starts_write[_i] = malloc(_BSIZE); \
        memset(mem_starts_write[_i], 'a', _BSIZE); \
        file_sizes[_i] = mem_sizes[_i] = _BSIZE; \
        file_starts[_i] = _foff; \
        _foff += _BSIZE; \
    } \
    zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &handle, &created, ZOIDFS_NO_OP_HINT); \
    zoidfs_lookup(NULL, NULL, fullpath_filename, &handle, ZOIDFS_NO_OP_HINT);\
    ret = zoidfs_write(&handle, mem_count, (const void **)mem_starts_write, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); \
    zoidfs_remove(NULL, NULL, fullpath_filename, NULL, ZOIDFS_NO_OP_HINT); \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        free(mem_starts_write[_i]); \
    } \
    return ret; \
}

#define ZOIDFS_READ_COMP_DEF(_N, _BSIZE) \
int _do_zoidfs_read_comp_nbuf##_N##_bsize##_BSIZE() \
{\
    int ret = 0; \
    size_t mem_sizes[_N]; \
    size_t _foff = 0; \
    size_t mem_count, file_count; \
    uint64_t file_sizes[_N], file_starts[_N]; \
    void *mem_starts_write[_N], *mem_starts_read[_N]; \
    size_t _i = 0; \
    zoidfs_handle_t handle; \
    int created = 0; \
    struct timeval now; \
    zoidfs_sattr_t sattr; \
    sattr.mask = ZOIDFS_ATTR_SETABLE; \
    sattr.mode = 0755; \
    sattr.uid = getuid(); \
    sattr.gid = getgid(); \
    gettimeofday(&now, NULL); \
    sattr.atime.seconds = now.tv_sec; \
    sattr.atime.nseconds = now.tv_usec; \
    sattr.mtime.seconds = now.tv_sec; \
    sattr.mtime.nseconds = now.tv_usec; \
    mem_count = _N; \
    file_count = _N; \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        mem_starts_write[_i] = malloc(_BSIZE); \
        mem_starts_read[_i] = malloc(_BSIZE); \
        memset(mem_starts_write[_i], 'a', _BSIZE); \
        memset(mem_starts_read[_i], '0', _BSIZE); \
        file_sizes[_i] = mem_sizes[_i] = _BSIZE; \
        file_starts[_i] = _foff; \
        _foff += _BSIZE; \
    } \
    zoidfs_create(&basedir_handle, component_filename, NULL, &sattr, &handle, &created, ZOIDFS_NO_OP_HINT); \
    zoidfs_lookup(&basedir_handle, component_filename, NULL, &handle, ZOIDFS_NO_OP_HINT);\
    zoidfs_write(&handle, mem_count, (const void **)mem_starts_write, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); \
    ret = zoidfs_read(&handle, mem_count, (void **)mem_starts_read, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); \
    zoidfs_remove(&basedir_handle, component_filename, NULL, NULL, ZOIDFS_NO_OP_HINT); \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        if(memcmp(mem_starts_write[_i], mem_starts_read[_i], _BSIZE) != 0) \
        { \
            ret = -1; \
        } \
        free(mem_starts_write[_i]); \
        free(mem_starts_read[_i]); \
    } \
    return ret; \
}

#define ZOIDFS_READ_FULL_DEF(_N, _BSIZE) \
int _do_zoidfs_read_full_nbuf##_N##_bsize##_BSIZE() \
{\
    int ret = 0; \
    size_t mem_sizes[_N]; \
    size_t _foff = 0; \
    size_t mem_count, file_count; \
    uint64_t file_sizes[_N], file_starts[_N]; \
    void *mem_starts_write[_N], *mem_starts_read[_N]; \
    size_t _i = 0; \
    zoidfs_handle_t handle; \
    int created = 0; \
    struct timeval now; \
    zoidfs_sattr_t sattr; \
    sattr.mask = ZOIDFS_ATTR_SETABLE; \
    sattr.mode = 0755; \
    sattr.uid = getuid(); \
    sattr.gid = getgid(); \
    gettimeofday(&now, NULL); \
    sattr.atime.seconds = now.tv_sec; \
    sattr.atime.nseconds = now.tv_usec; \
    sattr.mtime.seconds = now.tv_sec; \
    sattr.mtime.nseconds = now.tv_usec; \
    mem_count = _N; \
    file_count = _N; \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        mem_starts_write[_i] = malloc(_BSIZE); \
        mem_starts_read[_i] = malloc(_BSIZE); \
        memset(mem_starts_write[_i], 'a', _BSIZE); \
        memset(mem_starts_read[_i], '0', _BSIZE); \
        file_sizes[_i] = mem_sizes[_i] = _BSIZE; \
        file_starts[_i] = _foff; \
        _foff += _BSIZE; \
    } \
    zoidfs_create(NULL, NULL, fullpath_filename, &sattr, &handle, &created, ZOIDFS_NO_OP_HINT); \
    zoidfs_lookup(NULL, NULL, fullpath_filename, &handle, ZOIDFS_NO_OP_HINT);\
    zoidfs_write(&handle, mem_count, (const void **)mem_starts_write, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); \
    ret = zoidfs_read(&handle, mem_count, (void **)mem_starts_read, mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); \
    zoidfs_remove(NULL, NULL, fullpath_filename, NULL, ZOIDFS_NO_OP_HINT); \
    for(_i = 0 ; _i < mem_count ; _i++) \
    { \
        if(memcmp(mem_starts_write[_i], mem_starts_read[_i], _BSIZE) != 0) \
        { \
            ret = -1; \
        }\
        free(mem_starts_write[_i]); \
        free(mem_starts_read[_i]); \
    } \
    return ret; \
}

/*
 * invoke write macro
 */
#define ZOIDFS_WRITE_FULL(_N, _BSIZE) _do_zoidfs_write_full_nbuf##_N##_bsize##_BSIZE()
#define ZOIDFS_WRITE_COMP(_N, _BSIZE) _do_zoidfs_write_comp_nbuf##_N##_bsize##_BSIZE()

/*
 * invoke read macro
 */
#define ZOIDFS_READ_FULL(_N, _BSIZE) _do_zoidfs_read_full_nbuf##_N##_bsize##_BSIZE()
#define ZOIDFS_READ_COMP(_N, _BSIZE) _do_zoidfs_read_comp_nbuf##_N##_bsize##_BSIZE()

/*
 * write macro buffer size constants
 */
#define B_1M 1 * 1024 * 1024
#define B_2M 2 * 1024 * 1024
#define B_4M 4 * 1024 * 1024
#define B_8M 8 * 1024 * 1024

ZOIDFS_WRITE_COMP_DEF(1, B_1M)
ZOIDFS_WRITE_COMP_DEF(2, B_1M)
ZOIDFS_WRITE_COMP_DEF(3, B_1M)
ZOIDFS_WRITE_COMP_DEF(4, B_1M)
ZOIDFS_WRITE_COMP_DEF(8, B_1M)
ZOIDFS_WRITE_COMP_DEF(1, B_2M)
ZOIDFS_WRITE_COMP_DEF(2, B_2M)
ZOIDFS_WRITE_COMP_DEF(3, B_2M)
ZOIDFS_WRITE_COMP_DEF(4, B_2M)
ZOIDFS_WRITE_COMP_DEF(1, B_4M)
ZOIDFS_WRITE_COMP_DEF(2, B_4M)
ZOIDFS_WRITE_COMP_DEF(1, B_8M)

ZOIDFS_WRITE_FULL_DEF(1, B_1M)
ZOIDFS_WRITE_FULL_DEF(2, B_1M)
ZOIDFS_WRITE_FULL_DEF(3, B_1M)
ZOIDFS_WRITE_FULL_DEF(4, B_1M)
ZOIDFS_WRITE_FULL_DEF(8, B_1M)
ZOIDFS_WRITE_FULL_DEF(1, B_2M)
ZOIDFS_WRITE_FULL_DEF(2, B_2M)
ZOIDFS_WRITE_FULL_DEF(3, B_2M)
ZOIDFS_WRITE_FULL_DEF(4, B_2M)
ZOIDFS_WRITE_FULL_DEF(1, B_4M)
ZOIDFS_WRITE_FULL_DEF(2, B_4M)
ZOIDFS_WRITE_FULL_DEF(1, B_8M)

ZOIDFS_READ_COMP_DEF(1, B_1M)
ZOIDFS_READ_COMP_DEF(2, B_1M)
ZOIDFS_READ_COMP_DEF(3, B_1M)
ZOIDFS_READ_COMP_DEF(4, B_1M)
ZOIDFS_READ_COMP_DEF(8, B_1M)
ZOIDFS_READ_COMP_DEF(1, B_2M)
ZOIDFS_READ_COMP_DEF(2, B_2M)
ZOIDFS_READ_COMP_DEF(3, B_2M)
ZOIDFS_READ_COMP_DEF(4, B_2M)
ZOIDFS_READ_COMP_DEF(1, B_4M)
ZOIDFS_READ_COMP_DEF(2, B_4M)
ZOIDFS_READ_COMP_DEF(1, B_8M)

ZOIDFS_READ_FULL_DEF(1, B_1M)
ZOIDFS_READ_FULL_DEF(2, B_1M)
ZOIDFS_READ_FULL_DEF(3, B_1M)
ZOIDFS_READ_FULL_DEF(4, B_1M)
ZOIDFS_READ_FULL_DEF(8, B_1M)
ZOIDFS_READ_FULL_DEF(1, B_2M)
ZOIDFS_READ_FULL_DEF(2, B_2M)
ZOIDFS_READ_FULL_DEF(3, B_2M)
ZOIDFS_READ_FULL_DEF(4, B_2M)
ZOIDFS_READ_FULL_DEF(1, B_4M)
ZOIDFS_READ_FULL_DEF(2, B_4M)
ZOIDFS_READ_FULL_DEF(1, B_8M)

int testWRITE(void)
{
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(1, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(2, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(3, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(4, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(8, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(1, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(2, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(3, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(4, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(1, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(2, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_COMP(1, B_8M));

    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(1, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(2, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(3, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(4, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(8, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(1, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(2, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(3, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(4, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(1, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(2, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_WRITE_FULL(1, B_8M));

    return 0;
}

int testREAD(void)
{
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(1, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(2, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(3, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(4, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(8, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(1, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(2, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(3, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(4, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(1, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(2, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_COMP(1, B_8M));

    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(1, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(2, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(3, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(4, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(8, B_1M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(1, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(2, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(3, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(4, B_2M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(1, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(2, B_4M));
    CU_ASSERT(ZFS_OK == ZOIDFS_READ_FULL(1, B_8M));

    return 0;
}

int safe_read (const char * fullpath, 
      const zoidfs_handle_t * handle /* in:ptr */,
                size_t mem_count /* in:obj */,
                void * mem_starts[] /* out:arr2d:size=+1:zerocopy */,
                const size_t mem_sizes[] /* in:arr:size=-2 */,
                size_t file_count /* in:obj */,
                const uint64_t file_starts[] /* in:arr:size=-1 */,
                uint64_t file_sizes[] /* inout:arr:size=-2 */)
{
   assert (fullpath); 
   return zoidfs_read (handle, mem_count, mem_starts, 
         mem_sizes, file_count, file_starts, file_sizes, ZOIDFS_NO_OP_HINT); 
}

int safe_write (const char * fullpath, 
      const zoidfs_handle_t * handle /* in:ptr */,
                 size_t mem_count /* in:obj */,
                 const void * mem_starts[] /* in:arr2d:size=+1:zerocopy */,
                 const size_t mem_sizes[] /* in:arr:size=-2 */,
                 size_t file_count /* in:obj */,
                 const uint64_t file_starts[] /* in:arr:size=-1 */,
                 uint64_t file_sizes[] /* inout:arr:size=-2 */)
{
   assert (fullpath); 
   return zoidfs_write (handle, mem_count, mem_starts, mem_sizes, file_count, 
         file_starts, file_sizes, ZOIDFS_NO_OP_HINT); 
}


void testValidate ()
{
   zoidfs_handle_t handle; 
   int created; 
   zoidfs_sattr_t attr; 
   const unsigned int mb = 1024*1024; 
   const unsigned int filesize = 128*mb; 
   const unsigned int maxseg = 32; 
   const unsigned int maxsegsize = 4*mb;
   const unsigned int maxmem = 256*mb; 
   const unsigned int rounds = 16; 
   const unsigned int firstskip = 4*mb; 
   const unsigned int maxhole = 4*mb; 
   void * mem; 
   uint64_t * ofs; 
   uint64_t * size; 
   unsigned int round; 

   assert (firstskip <= filesize); 

   attr.mask = 0; 

   /* create fullpath_filename */
   CU_ASSERT_TRUE_FATAL(ZFS_OK == zoidfs_create (0, 0, fullpath_filename, &attr, &handle, &created, ZOIDFS_NO_OP_HINT)); 
   CU_ASSERT_TRUE(created); 

   mem = malloc (maxmem); 
   ofs = malloc (sizeof (uint64_t) * maxseg); 
   size = malloc (sizeof (uint64_t) * maxseg); 

   for (round=0; round<rounds; ++round)
   {
      unsigned int i; 
      uint64_t curofs = random () % firstskip; 
      uint64_t cursize; 
      size_t totalsize = 0;

      /* generate access list */ 
      const unsigned int segments = random () % maxseg; 
      unsigned int segcount = 0; 

      int ret; 
      char fill; 

      for (i=0; i<segments; ++i)
      {
         cursize = random () % maxsegsize; 
         curofs += random () % maxhole; 

         if (curofs > filesize)
            curofs = filesize; 

         if (curofs + cursize > filesize)
            cursize = filesize - curofs; 

         if (curofs && cursize)
         {
            ofs[segcount] = curofs; 
            size[segcount] = cursize; 
            totalsize += cursize; 
            ++segcount; 
         }
      }

      fill = random() % 256; 
      memset (mem, fill, totalsize); 

      ret = safe_write (fullpath_filename, &handle, 1, (const void **) &mem, &totalsize, segcount,
            ofs, size);
      CU_ASSERT_EQUAL (ret, ZFS_OK); 

      ret = safe_read (fullpath_filename, &handle, 1, &mem, &totalsize, segcount, 
            ofs, size); 
      CU_ASSERT_EQUAL (ret, ZFS_OK); 

      for (i=0; i<totalsize; ++i)
      {
         if ( ((const char *) mem)[i] != fill)
         {
            char buf[255]; 
            snprintf (buf, sizeof(buf), "Problem in data validation! Expected %i, got %i!",
                  (int) fill, (int) ((const char *)mem)[i]); 
            CU_FAIL (buf); 
            break; 
         }
      }
   }

   free (size); 
   free (ofs); 
   free (mem); 


   CU_ASSERT_TRUE (ZFS_OK == zoidfs_remove (0, 0, fullpath_filename, 0, ZOIDFS_NO_OP_HINT)); 
}

int init_suite_dispatch_basic(void)
{
    return 0;
}

int clean_suite_dispatch_basic(void)
{
    return 0;
}

/* The main() function for setting up and running the tests.
 * Returns a CUE_SUCCESS on successful running, another
 * CUnit error code on failure.
 */
int main(int argc, char ** argv)
{
   CU_pSuite pSuite = NULL;

   if(argc < 2)
   {
    fprintf(stderr, "incorrect cmd line args!\n");
    return -1;
   }

    zoidfs_init();

   /* setup path names for the tests */
   init_path_names(argv[1]);

   /* init the base handle for the mount point*/
   init_basedir_handle(argv[1]);

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Basic Dispatcher Test Suite", init_suite_dispatch_basic, clean_suite_dispatch_basic);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (
        (NULL == CU_add_test(pSuite, "test of zoidfs_write()", (CU_TestFunc) testWRITE)) ||
        (NULL == CU_add_test(pSuite, "test of zoidfs_read()", (CU_TestFunc) testREAD)) ||
        (NULL == CU_add_test(pSuite, "data validation read/write", (CU_TestFunc) testValidate))
      )
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();

   zoidfs_finalize();

   return CU_get_error();
}
