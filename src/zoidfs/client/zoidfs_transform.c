#include "zoidfs_transform.h"
extern int zlib_compress_hook (void * stream, void * source, size_t * length, void ** dest,
                               size_t * output_length, int close);

zoidfs_write_compress init_compression (char * type)
{
    int x;
    zoidfs_write_compress comp;

    compress_init (type,&comp.compression_struct);
    if (strcmp("zlib",type) == 0)
    { 
        comp.transform = &zlib_compress_hook;
    }               
    return comp;
}

int zoidfs_transform (zoidfs_write_compress compression, void * input, 
                      size_t * input_length, void * output, size_t * buf_len,
                      int flush)
{
    size_t prev_position = *buf_len;
    int ret;

    if (compression.intern_buf == output)
        output += compression.buf_position;
    else
    {
        compression.intern_buf = output;
        compression.buf_position = 0;
    }

    while (*input_length > 0 && *buf_len > 0)
    {
        ret = compression.transform (compression.compression_struct, input, input_length,
                                     output, buf_len, flush);
        compression.buf_position += (prev_position - *buf_len);
        prev_position += *buf_len;
        output += compression.buf_position;
        if (ret != Z_OK)
            return ret;
    }
    
    if (*input_length > 0)
        return ZOIDFS_OUTPUT_FULL;
    else if (*buf_len > 0)
        return ZOIDFS_CONT;
    
    return ZOIDFS_CONT;
}

