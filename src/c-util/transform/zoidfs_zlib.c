#include "zoidfs_transform.h"
int compress_init (char * type, int level, void ** library)
{
    int ret; 
    void ** ret_value;
    
    if (strcmp(type,"zlib") == 0)
    {
        /* What level of compression to user (static for now) */
        int level = -1;

        /* Create the required zlib structure */
        z_stream * tmp = malloc(sizeof(z_stream));
        (*tmp).zalloc = Z_NULL;
        (*tmp).zfree = Z_NULL;
        (*tmp).opaque = Z_NULL;

        /* set mode to compress with compression level */
        ret = deflateInit(tmp, level);

        /* check if everything is ok */
        if (ret != Z_OK)
            return ret;
        ret_value = (void *)tmp;
    }
    /* No other compressions in use, exiting */
    else
    {
        return -1;
    }
    
    /* Set the library pointer to point to the struct */
    (*library) = ret_value;     
    return 0;
}

int decompress_init (char * type, void ** library)
{
    int ret; 
    void ** ret_value;
    
    if (strcmp(type,"zlib") == 0)
    {
        /* Create the required zlib structure */
        z_stream * tmp = malloc(sizeof(z_stream));
        (*tmp).zalloc = Z_NULL;
        (*tmp).zfree = Z_NULL;
        (*tmp).opaque = Z_NULL;
        (*tmp).avail_in = 0;
        (*tmp).next_in = Z_NULL;
        /* set mode to compress with compression level */
        ret = inflateInit(tmp);

        /* check if everything is ok */
        if (ret != Z_OK)
        {
            printf("ERROR");
            return ret;
        }

        ret_value = (void *)tmp;
    }
    else
    {
        return -1;
    }
    /* Set the library pointer to point to the struct */
    (*library) = ret_value;     
    return 0;    
}

int zlib_compress_hook (void * stream, void * source, size_t * length, void ** dest,
                        size_t * output_length, int close)
{
    return zlib_compress((z_stream *)stream, source, length, dest, output_length, close);
}
int zlib_compress (z_stream * stream, void ** source, size_t * length, void ** dest,
                   size_t * output_length, int close)
{
    int ret;
    size_t have;
    /* Malloc the buffer that will recieve the compress data */
    void * input = (*source);
    void * finished = (*dest); 
    z_stream * strm = stream;
    /* Setup the zlib buffers */
    (*strm).avail_in = *length;
    (*strm).next_in = input;
    (*strm).avail_out = *output_length;
    (*strm).next_out = finished;
    //fprintf(stderr,"zlib output_left: %i, input_left: %i\n",(*strm).avail_out,(*strm).avail_in);
    /* Compress the data */
    if (close == Z_FINISH || close == ZOIDFS_CLOSE )
    { 
        ret = deflate(strm, Z_FINISH );
        if (ret == Z_STREAM_END)
        {
            (void)deflateEnd(strm); 
            ret = ZOIDFS_STREAM_END;
        }
    }
    else if (close == Z_FULL_FLUSH || close == ZOIDFS_FLUSH)
    {
        ret = deflate(strm,Z_FULL_FLUSH);
    }
    else 
    {
        ret = deflate(strm, Z_NO_FLUSH );
    }
    /* Figure out how much of the output buffer has been used */
    finished += (*output_length - (*strm).avail_out);
    //input += (*length - (*strm).avail_in); 
    *output_length = (*strm).avail_out;
    *length = (*strm).avail_in;

    /* set the ourput buffer */
    (*dest) = finished;
    (*source) = input;
    if (ret == Z_BUF_ERROR)
        return ZOIDFS_BUF_ERROR;
    return ret;
}
int zlib_decompress (void * source, size_t * length, void ** dest,
                     size_t * output_length, z_stream * stream, int close)
{
    int ret;
    size_t have;
    /* Malloc the buffer that will recieve the compress data */
    void * finished = (*dest); 
    z_stream * strm = stream;
    /* Setup the zlib buffers */
    (*strm).avail_in = *length;
    (*strm).next_in = source;
    (*strm).avail_out = *output_length;
    (*strm).next_out = finished;
    /* Compress the data */
    if (close)
    { 
        ret = inflate(strm, Z_FINISH );
        if (ret == Z_STREAM_END)
        {
            (void)deflateEnd(strm); 
        }
    }
    else
    {
        ret = inflate(strm,  Z_SYNC_FLUSH );
    }

    /* Figure out how much of the output buffer has been used */
    have =  *output_length - (*strm).avail_out;
    *output_length = have;
    *length = (*strm).total_in;

    /* set the ourput buffer */
    (*dest) = finished;

    return ret;
}
