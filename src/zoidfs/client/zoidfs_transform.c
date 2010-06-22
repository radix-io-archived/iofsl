#include "zoidfs_transform.h"
extern int zlib_compress_hook (void * stream, void * source, size_t * length, void ** dest,
                               size_t * output_length, int close);

int zoidfs_transform_init (char * type, zoidfs_write_compress * comp)
{
    int x;
    (*comp).intern_buf = NULL;
    (*comp).buf_position = 1000;

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
    size_t max_buf = *buf_len;
    int ret;

    if ((*compression).intern_buf == output)
        output += (*compression).buf_position;
    else
    {
        (*compression).intern_buf = &output;
        (*compression).buf_position = (size_t)0;
    }
    fprintf(stderr,"NEW RUN\n",*input_length, *buf_len);    
    while ((int)(*compression).buf_position < (int)max_buf && (int)*input_length > 0)
    {
        ret = (*compression).transform ((*compression).compression_struct, input, input_length,
                                     output, buf_len, flush);
        (*compression).buf_position += *buf_len;
        //output += (*compression).buf_position;
        if (ret != Z_OK && ret != Z_STREAM_END)
            return ret;
        else if (ret == Z_STREAM_END)  
            return ZOIDFS_COMPRESSION_DONE;       
    }
    
    if (*input_length > 0)
        return ZOIDFS_OUTPUT_FULL;
    else if (*buf_len > 0)
        return ZOIDFS_CONT;
    return ZOIDFS_CONT;
}

