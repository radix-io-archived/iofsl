#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "iofwd_config.h"
#ifdef HAVE_ZLIB
#include "zlib.h"
#endif
#define SET_BINARY_MODE(file)
#define ZOIDFS_TRANSFORM_ERROR  -9
#define ZOIDFS_BUF_ERROR        -3
#define ZOIDFS_OUTPUT_FULL      -2
#define ZOIDFS_BUF_FULL         -1
#define ZOIDFS_CONT             0
#define ZOIDFS_COMPRESSION_DONE 1
#define ZOIDFS_STREAM_END       2
#define ZOIDFS_FLUSH            3
#define ZOIDFS_CLOSE            4
typedef struct
{
    int(*transform)(void *, void **, size_t *, void **, size_t *, int);
    void * compression_struct;
    void * type;
    void * intern_buf;
    size_t buf_position;
} zoidfs_write_compress;

