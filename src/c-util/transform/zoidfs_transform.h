#ifndef C_UTIL_TRANSFORM_ZOIDFS_TRANSFORM_H
#define C_UTIL_TRANSFORM_ZOIDFS_TRANSFORM_H

#define HAVE_LZF
#include <stdio.h>
#include <string.h>

#ifdef HAVE_LZF
#include "lzf/lzf.h"
#include "lzf/lzfP.h"
#include <rpc/xdr.h>
#include "zoidfs_lzf.h"
#include "zoidfs.h"
#define LZF_BUFF_SIZE 64000
typedef struct
{
  void * out_buf;     /* Compressed output buffer yet to be written */
  size_t out_buf_pos; /* Output buffer position */
  size_t out_buf_size;/* Maximum size of the output buffer */
  size_t out_buf_len; /* Current length of written output buffer */
  char out_buf_type;
  void * in_buf;
  size_t in_buf_pos;
  size_t in_buf_size;
  size_t in_buf_len;
  
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
//#include <bzlib.h>
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
  /* transform varibles */
  int(*transform)(void *, void **, size_t *, void **, size_t *, int);
  void * compression_struct;
  void * type;
  void * intern_buf;
  size_t buf_position;
  size_t total_in;


} zoidfs_write_compress;

typedef struct
{
  void * transform;
  char * type;
  int(*decompress)(void *, void **, size_t *, void **, size_t *, int);
} zoidfs_decompress;



int zoidfs_transform_init (char * type, zoidfs_write_compress * comp);
int zoidfs_transform (zoidfs_write_compress * compression, void ** input, 
                      size_t * input_length, void ** output, size_t * output_len,
                      int flush);

int zoidfs_transform_write_request (zoidfs_write_compress * transform,
				    zoidfs_write_vars * write_buffs,
				    size_t max_buff_size,
				    size_t num_of_buffs_to_fill,
				    void *** buffer,
				    size_t ** buffer_sizes,
				    size_t * buf_count,
				    size_t * total_len,
				    int * close
				    );
int zoidfs_transform_decompress_init (char * type_str, zoidfs_decompress * comp);
int zoidfs_transform_read_request (zoidfs_decompress * transform, zoidfs_read_vars * read_buffs, size_t * outputs_filled, int close);
void zoidfs_transform_destroy (zoidfs_write_compress * comp);
void zoidfs_transform_change_transform (char * type, zoidfs_write_compress * comp);

#endif

