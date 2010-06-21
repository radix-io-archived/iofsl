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

typedef struct
{
    void * comp_struct;          /* Holds the compression struct information */
    void ** input_buffers;       /* Buffers to be compressed                 */
    size_t * input_buf_len;     /* Amount of data left in each buffer       */
    size_t  input_num_buffers;   /* Number of input buffers                  */
    void *  output_buf;          /* Output buffer                            */
    size_t  output_buf_size;     /* Output buffer size                       */ 
    size_t  pipeline_size;       /* Size of the pipeline (maximum buff size) */   
    size_t * input_remaining;    /* Array containing the amount of data still 
                                    left to be read by the compression       */
    size_t current_buf;          /* Current input buffer to read from        */
} zoidfs_write_compress;
