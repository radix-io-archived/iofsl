#include "zoidfs_transform.h"
extern int zlib_compress_hook (void * stream, void * source, size_t * length, void ** dest,
                               size_t * output_length, int close);

int zoidfs_transform_init (char * type, zoidfs_write_compress * comp)
{
    int x;
    (*comp).intern_buf = NULL;
    (*comp).buf_position = 0;

    compress_init (type,&(*comp).compression_struct);
    if (strcmp("zlib",type) == 0)
    { 
        (*comp).transform = &zlib_compress_hook;
    }               
    return 0;
}

int zoidfs_transform (zoidfs_write_compress * compression, void * input, 
                      size_t * input_length, void * output, size_t * buf_len,
                      int flush)
{
    int ret;
    size_t total_input = (*compression).buf_position + *input_length;
    size_t output_buf_left = *buf_len;
    size_t tmp;
    while (ret != Z_BUF_ERROR)
    {  
        ret = (*compression).transform ((*compression).compression_struct, 
                                        input, input_length, output, buf_len, 
                                        flush);       
        tmp = *input_length;
        *input_length = total_input - *input_length;
        if ((int)*input_length < 0)
            *input_length = 0;
        output_buf_left = output_buf_left - *buf_len;
        *buf_len =  output_buf_left;
        if (ret != Z_OK && ret != Z_STREAM_END && ret != Z_BUF_ERROR)
            return ret;
        else if (ret == Z_STREAM_END)  
            return ZOIDFS_COMPRESSION_DONE;       
    }
    if ((int)*input_length > 0)
        return ZOIDFS_OUTPUT_FULL;
    else if (*buf_len > 0)
        return ZOIDFS_CONT;
    return ZOIDFS_CONT;
}

