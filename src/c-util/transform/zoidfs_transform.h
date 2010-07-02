
#define HAVE_LZF
#include <stdio.h>
#include <string.h>
#ifdef HAVE_LZF
#include "lzf/lzf.h"
#include "lzf/lzfP.h"
#include <rpc/xdr.h>
#include "zoidfs_lzf.h"

#define LZF_BUFF_SIZE 64000
typedef struct
{
    void * buf;
    int cur_position;
    int buf_size;
} lzf_state_var;

#endif
#include <stdlib.h>
#include <assert.h>
#include "iofwd_config.h"
#ifdef HAVE_ZLIB
#include "zlib.h"
#include "zoidfs_zlib.h"
#endif
#ifdef HAVE_BZLIB
#include "bzlib.h"
#include "zoidfs_bzip.h"
#endif
#include "zoidfs_lzf.h"
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
    size_t total_in;
} zoidfs_write_compress;

int zoidfs_transform_init (char * type, zoidfs_write_compress * comp);
int zoidfs_transform (zoidfs_write_compress * compression, void ** input, 
                      size_t * input_length, void ** output, size_t * output_len,
                      int flush);
