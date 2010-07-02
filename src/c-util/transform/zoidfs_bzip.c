#include "zoidfs_transform.h"

int bzip_compress_init (void ** library, size_t total_in)
{
    int ret; 
    void ** ret_value;
    /* Create the required zlib structure */

    bz_stream * tmp = malloc(sizeof(z_stream));
    ret = BZ2_bzCompressInit(bz_stream, 9, 0, 0);
    (*tmp).avail_in = total_in;
    /* check if everything is ok */
    if (ret != BZ_OK)
        return ret;

    ret_value = (void *)tmp;
    
    (*library) = ret_value;     
    return 0;
}

int bzip_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close)
{
    return bzip_compress((z_stream *)stream, source, length, dest, output_length, close);
}
int bzip_compress (z_stream * stream, void ** source, size_t * length, void ** dest,
                   size_t * output_length, int close)
{
    int ret;
    /* Malloc the buffer that will recieve the compress data */
    void * input = (*source);
    void * finished = (*dest); 
    bz_stream * strm = stream;
    /* Setup the zlib buffers */
    size_t store = (*strm).avail_in;
    (*strm).next_in = input;
    (*strm).avail_out = *output_length;
    (*strm).next_out = finished;
    //fprintf(stderr,"zlib output_left: %i, input_left: %i\n",(*strm).avail_out,(*strm).avail_in);
    /* Compress the data */
    if (close == Z_FINISH || close == ZOIDFS_CLOSE )
    { 
        ret = BZ2_bzCompress(strm, BZ_FINISH );
        if (ret == Z_STREAM_END)
        {
            (void)BZ2_bzCompressEnd(strm); 
            ret = ZOIDFS_STREAM_END;
        }
    }
    else if (close == BZ_FULL_FLUSH || close == ZOIDFS_FLUSH)
    {
        ret = BZ2_bzCompress(strm,BZ_FLUSH);
    }
    else 
    {
        ret = BZ2_bzCompress(strm, BZ_RUN );
    }
    /* Figure out how much of the output buffer has been used */
    finished += (*output_length - (*strm).avail_out);
    //input += (*length - (*strm).avail_in); 
    *output_length = (*strm).avail_out;
    *length = *length - ((*strm).avail_in - store);
    /* set the ourput buffer */
    (*dest) = finished;
    (*source) = input;
    if ((*length) == 0 || (*output_length) == 0)
        return ZOIDFS_BUF_ERROR;
    else (ret == BZ_SEQUENCE_ERROR || ret == BZ_PARAM_ERROR)
        return ZOIDFS_TRANSFORM_ERROR;
    return ZOIDFS_CONT;
}
