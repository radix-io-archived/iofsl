#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include "zlib.h"
#define ZLIB_INIT = 0
#define ZLIB_CONT = 1
#define ZLIB_CLOSE = 2
#define SET_BINARY_MODE(file)
#define ZOIDFS_BUF_FULL         42
#define ZOIDFS_COMPRESSION_DONE 41
#define ZOIDFS_CONT             40
#define ZOIDFS_OUTPUT_FULL      43
#define ZOIDFS_FLUSH            500
typedef struct
{
    int(*transform)(void *, void *, size_t *, void **, size_t *, int);
    void * compression_struct;
    void * intern_buf;
    size_t buf_position;
} zoidfs_write_compress;

