#include "zoidfs_transform.h"

extern int zlib_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                               size_t * output_length, int close);

int passthrough (void * stream, void ** source, size_t * length, void ** dest,
                 size_t * output_length, int close)
{
    (*dest) = (*source);
    if (*output_length > *length)
    {
        (*source) += *length;
        *output_length =  *output_length - *length;
        (*dest) += *length;
        *length = 0;
    }
    else
    {
        (*source) += *output_length;
        *length = *length - *output_length;        
        (*dest) += *output_length;
        *output_length = 0;           
    } 
    return ZOIDFS_BUF_ERROR;
}

int zoidfs_transform_init (char * type, zoidfs_write_compress * comp)
{
    int x;
    int level = -1;
    (*comp).intern_buf = NULL;
    (*comp).buf_position = 0;
    (*comp).type = type;
    if (strcmp("ZLIB:",type) == 0)
    { 
        #ifdef HAVE_ZLIB
            compress_init ("zlib",level,&(*comp).compression_struct);
            (*comp).transform = &zlib_compress_hook;
        #else
            printf("Error! Zlib library is not availible!\n");
            return -1;
        #endif
    }   
    else if (strcmp("passthrough",type) == 0)
    {
        (*comp).transform = &passthrough;                    
    }
    return 0;
}

int zoidfs_transform_change_transform (char * type, zoidfs_write_compress * comp)
{
    int level = -1;
    if (strcmp("zlib",type) == 0)
    { 
        #ifdef HAVE_ZLIB
        compress_init (type,level,&(*comp).compression_struct);
        (*comp).transform = &zlib_compress_hook;
        #else
            printf("Error! Zlib library is not availible!\n");
            return -1;
        #endif        
    }      
    else if (strcmp("passthrough",type) == 0)
    {
        (*comp).transform = &passthrough;                    
    }
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
    while (ret != ZOIDFS_BUF_ERROR)
    {  
        /* Compress's the file based on some sort of compression scheme 
           defined in the initialization function */
        ret = (*compression).transform ((*compression).compression_struct, 
                                        input, input_length, output, output_len, 
                                        flush);             
        if (ret == ZOIDFS_STREAM_END)  
            return ZOIDFS_COMPRESSION_DONE; 
        else if (ret == ZOIDFS_TRANSFORM_ERROR)
            return ret;      
    }
    
    if ((int)*input_length > 0)
        return ZOIDFS_OUTPUT_FULL;
    else if (*output_len > 0)
        return ZOIDFS_CONT;
    return ZOIDFS_CONT;
}
