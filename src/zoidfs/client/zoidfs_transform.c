#include "zoidfs_transform.h"
extern int zlib_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                               size_t * output_length, int close);

int zoidfs_transform_init (char * type, zoidfs_write_compress * comp)
{
    int x;
    int level = 0;
    (*comp).intern_buf = NULL;
    (*comp).buf_position = 0;
    (*comp).type = type;
    compress_init (type,level,&(*comp).compression_struct);
    if (strcmp("zlib",type) == 0)
    { 
        (*comp).transform = &zlib_compress_hook;
    }               
    return 0;
}

void zoidfs_transform_destroy (zoidfs_write_compress * comp)
{
    if (strcmp((*comp).type,"zlib") == 0)
    {
        //(void)deflateEnd((*comp).compression_struct);
    }
} 

int zoidfs_transform (zoidfs_write_compress * compression, void ** input, 
                      size_t * input_length, void ** output, size_t * output_len,
                      int flush)
{
    int ret = 0;
    while (ret != Z_BUF_ERROR)
    {  
        /* Compress's the file based on some sort of compression scheme 
           defined in the initialization function */
        ret = (*compression).transform ((*compression).compression_struct, 
                                        input, input_length, output, output_len, 
                                        flush); 
        //fprintf(stderr, "input_len: %i, output_len: %i, input ptr: %i, output ptr: %i ret: %i\n",
        //                *input_length, *output_len, *input, *output,ret);
            
        if (ret == Z_STREAM_END)  
            return ZOIDFS_COMPRESSION_DONE;       
    }
    
    if ((int)*input_length > 0)
        return ZOIDFS_OUTPUT_FULL;
    else if (*output_len > 0)
        return ZOIDFS_CONT;
    return ZOIDFS_CONT;
}

