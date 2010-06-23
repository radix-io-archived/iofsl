#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#ifdef HAVE_ZLIB
#include "zlib.h"
#endif
#define SET_BINARY_MODE(file)
#define ZOIDFS_BUF_FULL         42
#define ZOIDFS_COMPRESSION_DONE 41
#define ZOIDFS_CONT             0
#define ZOIDFS_OUTPUT_FULL      43
#define ZOIDFS_FLUSH            500
#define ZOIDFS_CLOSE            501
#define ZOIDFS_BUF_ERROR        -5
#define ZOIDFS_STREAM_END       2
typedef struct
{
    int(*transform)(void *, void **, size_t *, void **, size_t *, int);
    void * compression_struct;
    void * type;
    void * intern_buf;
    size_t buf_position;
} zoidfs_write_compress;

