#include "zoidfs_transform.h"
int zlib_compress (void * source, size_t * length, void ** dest,
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
    if (close = Z_FINISH)
    { 
        ret = deflate(strm, Z_FINISH );
        if (ret == Z_STREAM_END)
        {
            (void)deflateEnd(&strm); 
        }
    }
    else if (close == Z_FULL_FLUSH)
    {
        ret = deflate(strm,Z_FULL_FLUSH);
    }
    else 
    {
        ret = deflate(strm, Z_NO_FLUSH );
    }
    /* Figure out how much of the output buffer has been used */
    have =  *output_length - (*strm).avail_out;
    *output_length = have;
    *length = (*strm).total_in;

    /* set the ourput buffer */
    (*dest) = finished;
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
