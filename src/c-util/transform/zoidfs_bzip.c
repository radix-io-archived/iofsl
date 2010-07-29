#include "zoidfs_transform.h"
int bzip_decompress_init (void ** transform)
{
  int ret;
  bz_stream * tmp = malloc(sizeof(bz_stream));
  (*tmp).bzalloc = Z_NULL;
  (*tmp).bzfree = Z_NULL;
  (*tmp).opaque = Z_NULL;
  ret = BZ2_bzDecompressInit(tmp, 0, 0);
  if (ret != BZ_OK)
    return -1;
  (*transform) = (void *) tmp;
  return 0;
}

int bzip_compress_init (char * type, int block_size, int verbosity, 
                        int work_factor,  void ** library)
{
    int ret; 
    void ** ret_value;
    if (strcmp(type,"bzip") == 0)
    {
        /* Create the required zlib structure */
        bz_stream * tmp = malloc(sizeof(bz_stream));
	tmp->bzalloc = NULL;
	tmp->bzfree = NULL;
	tmp->opaque = NULL;
        /* set mode to compress with compression level */
        ret = BZ2_bzCompressInit(tmp, block_size, verbosity, work_factor);
        
        /* check if everything is ok */
        if (ret != BZ_OK)
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

int bzip_compress_hook (void * stream, void ** source, size_t * length, 
                        void ** dest, size_t * output_length, int close)
{
    return bzip_compress((bz_stream *)stream, source, length, dest, 
                         output_length, close);
}
int bzip_compress (bz_stream * stream, void ** source, size_t * length, 
                   void ** dest, size_t * output_length, int close)
{
    int ret;

    /* Malloc the buffer that will recieve the compress data */
    void * input = (*source);
    void * finished = (*dest); 
    bz_stream * strm = stream;
    
    if ((*length) == 0 && close != ZOIDFS_CLOSE)
      return ZOIDFS_BUF_ERROR;
    /* Setup the zlib buffers */
    (*strm).avail_in = *length;
    (*strm).next_in = input; 
    (*strm).avail_out = *output_length;
    (*strm).next_out = finished;

    /* Compress the data */
    if (close == BZ_FINISH || close == ZOIDFS_CLOSE )
    { 
        ret = BZ2_bzCompress(strm, BZ_FINISH );
        if (ret == BZ_STREAM_END)
        {
            (void)BZ2_bzCompressEnd(strm); 
            ret = ZOIDFS_STREAM_END;
        }
    }
    else if (close == ZOIDFS_FLUSH)
    {
        ret = BZ2_bzCompress(strm, BZ_FLUSH);
    }
    else 
    {
        ret = BZ2_bzCompress(strm, BZ_RUN);
    }

    /* Figure out how much of the output buffer has been used */
    finished += (*output_length - (*strm).avail_out);
    input += (*length - (*strm).avail_in); 
    *output_length = (*strm).avail_out;
    *length = (*strm).avail_in;

    /* set the ourput buffer */
    (*dest) = finished;
    (*source) = input;

    if (ret == BZ_RUN_OK)
        return ZOIDFS_BUF_ERROR;
    else if (ret != BZ_RUN && ret != ZOIDFS_STREAM_END)
      {
	fprintf(stderr,"Returning error\n");
	assert(1 == 0);
        return ZOIDFS_TRANSFORM_ERROR;
      }
    return ret;
}

int bzip_decompress (bz_stream * stream, void ** source,
		     size_t * length, void ** dest,
                     size_t * output_length, int close)
{
    int ret;
    void * input = (*source);
    void * finished = (*dest);
    bz_stream * strm = stream;
    /* Setup the zlib buffers */
    if ((*output_length) == 0 )
    	return ZOIDFS_BUF_ERROR;
    (*strm).avail_in = *length;
    (*strm).next_in =  input;
    (*strm).avail_out = *output_length;
    (*strm).next_out = finished;
    /* Compress the data */
    if (close)
    { 
        ret = BZ2_bzDecompress(strm);
        if (ret == Z_STREAM_END)
        {
            (void)BZ2_bzDecompressEnd(strm); 
        }
    }
    else
    {
        ret = BZ2_bzDecompress(strm);
    }

    /* Figure out how much of the output buffer has been used */
    finished += (*output_length - (*strm).avail_out);
    input += (*length - (*strm).avail_in);
    *output_length = (*strm).avail_out;
    *length = (*strm).avail_in;

    /* set the ourput buffer */
    (*dest) = finished;
    (*source) = input;

    if (ret == BZ_STREAM_END)
    	return ZOIDFS_STREAM_END;
    else if (ret != BZ_OK && ret != ZOIDFS_STREAM_END)
        return ZOIDFS_TRANSFORM_ERROR;
    return ret;
}

