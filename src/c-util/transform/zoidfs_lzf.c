#include "zoidfs_transform.h"
#define ZOIDFS_DECOMPRESSION_DONE 11
/* NOTE: Due to the way this compression works. The data stream
   produced by lzf_compress will contain multiple independant
   compressed blocks of size LZF_BUFF_SIZE. These blocks are
   proceded by a charactor and an XDR_INT describing whether or
   not the chunk is compressed and the size of the  compressed
   or uncompressed chunk.

   Data Layout:
        Byte 1: Charactor (U for uncompressed, C for Compressed
                           chunk)
        Byte 2-sizeof(XDR_INT): Encoded's how many bytes are
                                contained in this chunk
        Remaining bytes contain the data (Until XDR_INT is reached)

*/

int lzf_compress_init (void ** library, size_t buffer_size)
{
    lzf_state_var * tmp = malloc(sizeof(lzf_state_var));
    int xdr_header_size;
    xdr_header_size = xdr_sizeof((xdrproc_t)xdr_int, &xdr_header_size);  
    (*tmp).out_buf = malloc(LZF_BUFF_SIZE + xdr_header_size);
    (*tmp).out_buf_size = LZF_BUFF_SIZE;
    (*tmp).out_buf_len = 0;
    (*tmp).out_buf_pos = 0;
    (*tmp).in_buf = malloc(LZF_BUFF_SIZE + xdr_header_size);
    (*tmp).in_buf_size = LZF_BUFF_SIZE;
    (*tmp).in_buf_len = 0;
    (*tmp).in_buf_pos = 0;
    (*tmp).out_buf_type = 'U';
    (*library) = tmp;     
    return 0;
}

int lzf_decompress_init (void ** library)
{
    lzf_state_var * tmp = malloc(sizeof(lzf_state_var));
    int xdr_header_size;
    xdr_header_size = xdr_sizeof((xdrproc_t)xdr_int, &xdr_header_size);  
    (*tmp).out_buf = malloc(LZF_BUFF_SIZE + xdr_header_size);
    (*tmp).out_buf_size = LZF_BUFF_SIZE;
    (*tmp).out_buf_len = 0;
    (*tmp).out_buf_pos = 0;
    (*tmp).in_buf = malloc(LZF_BUFF_SIZE + xdr_header_size);
    (*tmp).in_buf_size = LZF_BUFF_SIZE;
    (*tmp).in_buf_len = 0;
    (*tmp).in_buf_pos = 0;
    (*tmp).out_buf_type = 'U';
    (*library) = tmp;     
    return 0;
}



inline int lzf_compress_block (lzf_state_var * state, void ** input, 
			       size_t * in_length, int close)
{
  int ret = 0;
  state->out_buf_type = 'C';
  /* if there is some uncompressed input buffer remaining */
  if (state->in_buf_len > 0)
    {
      /* if there is enough data to create a block */
      if ((*in_length) + state->in_buf_len >= LZF_BUFF_SIZE)
	{
	  memcpy(state->in_buf, (*input), LZF_BUFF_SIZE - state->in_buf_len);
	  (*in_length) -= LZF_BUFF_SIZE - state->in_buf_len;
	  (*input) += LZF_BUFF_SIZE - state->in_buf_len;

	  state->in_buf -= state->in_buf_pos;
	  ret = lzf_compress (state->in_buf, LZF_BUFF_SIZE, state->out_buf, LZF_BUFF_SIZE);	
	  if (ret == 0)
	    {
	      memcpy(state->out_buf, state->in_buf, LZF_BUFF_SIZE);
	      ret = LZF_BUFF_SIZE;
	      state->out_buf_type = 'U';
	    }
	  state->out_buf_len = ret;
	  state->in_buf_len = 0;
	  state->in_buf_pos = 0;
	  return ret;
	}
      /* If the close flag is on, compress final block and write out */
      else if (close == ZOIDFS_CLOSE)
	{
	  memcpy(state->in_buf, (*input), (*in_length));
	  (*input) += (*in_length);

	  state->in_buf -= state->in_buf_pos;
	  ret = lzf_compress (state->in_buf, state->in_buf_pos + (*in_length),
			      state->out_buf, LZF_BUFF_SIZE);
	  if (ret == 0)
	    {
	      memcpy(state->out_buf, state->in_buf, state->in_buf_pos + (*in_length));
	      ret = state->in_buf_pos + (*in_length);
	      state->out_buf_type = 'U';
	    }
	  state->out_buf_len = ret;
	  state->in_buf_len = 0;
	  state->in_buf_pos = 0;
	  (*in_length) = 0;

	  return ZOIDFS_COMPRESSION_DONE;	  
	}
      else
	{
	  memcpy(state->in_buf, (*input), (*in_length));
	  state->in_buf_len += (*in_length);
	  state->in_buf += (*in_length);
	  state->in_buf_pos += (*in_length);
	  (*input) += (*in_length);
	  (*in_length) = 0;
	  return ZOIDFS_BUF_ERROR;
	}
    }
  else
    {
      /* If there is enough input to consume one full block */
      if ((*in_length) > LZF_BUFF_SIZE)
	{
	  ret = lzf_compress((*input), LZF_BUFF_SIZE, state->out_buf, LZF_BUFF_SIZE);
	  if (ret == 0)
	    {
	      memcpy(state->out_buf, state->in_buf, LZF_BUFF_SIZE);
	      ret = LZF_BUFF_SIZE;
	      state->out_buf_type = 'U';
	    }
	  (*input) += LZF_BUFF_SIZE;
	  (*in_length) -= LZF_BUFF_SIZE;
	  state->out_buf_len = ret;
	  state->out_buf_pos = 0;
	  return ret;
	}
      /* else copy it to a buffer for a another trial */
      else
	{
	  memcpy(state->in_buf, (*input), (*in_length));
	  state->in_buf_len += (*in_length);
	  state->in_buf += (*in_length);
	  state->in_buf_pos += (*in_length);
	  (*input) += (*in_length);
	  (*in_length) = 0;
	  return ZOIDFS_BUF_ERROR;
	}
    }
  return ZOIDFS_TRANSFORM_ERROR;
}
 inline int lzf_dump_comp_buffer(lzf_state_var * state, void ** output, size_t * output_left)
{
  size_t pos_adj;
  int out_buf_rem = state->out_buf_len - state->out_buf_pos;
  /* Writes out any output buffer (compressed buffer) that could not be 
     placed into output last time due to output buffer being full */

  if (state->out_buf_len > 0)
    {
      /* If there is enough output to write the full stored buffer */
      if (  (int)((*output_left) - out_buf_rem)  > 0)
      {
	  /* Make the copy */
	  memcpy((*output), state->out_buf, out_buf_rem);
	  (*output_left) -= out_buf_rem;
	  (*output) += out_buf_rem;
	  
	  /* Reset the output buffer state */
	  state->out_buf_len = 0;
	  state->out_buf -= state->out_buf_pos;
	  state->out_buf_pos = 0;
	  return 0;
	}
      /* else: write out what you can */
      else
	{
	  /* how much data to write out to the output buffer */
	  pos_adj = (*output_left);

	  /* copy what we can to the output buffer */
	  memcpy((*output),state->out_buf, pos_adj);
	  (*output) += pos_adj;
	  (*output_left) = 0;

	  /* Adjust the state values to reflect how much data is remaining */
	  state->out_buf += pos_adj;
	  state->out_buf_pos += pos_adj;
	  return ZOIDFS_OUTPUT_FULL;
	}
    }
  return 0;
}

inline void lzf_compress_gen_header (lzf_state_var * state, int len)
{
  XDR    xdr_header_data;
  int    xdr_header_size = len;    
  char * xdr_buffer = malloc(len * sizeof(char)); 
  xdrmem_create(&xdr_header_data,xdr_buffer,xdr_header_size,XDR_ENCODE); 
  xdr_int(&xdr_header_data,&state->out_buf_len);
  ((char *)state->out_buf)[0] = state->out_buf_type;
  state->out_buf += 1;
  memcpy(state->out_buf, xdr_buffer, xdr_header_size);
  state->out_buf -= 1;
  state->out_buf_len += 5;

}

int lzf_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close)
{
    return lzf_compress_data((lzf_state_var *)stream, source, length, dest, output_length, close);
}
int lzf_compress_data (lzf_state_var * stream, void ** source, size_t * length, void ** dest,
                       size_t * output_length, int close)
{
  size_t lzf_block_size = LZF_BUFF_SIZE;
  
  int count = 0;
  int comp_ret = 0;
  int ret = 0;
  /* Sets up the xdr encoding */
   
  XDR    xdr_header_data;
  int    xdr_header_size = 0;    
  char * xdr_buffer; 

  /* Set up the header size and internal buffer size */
  if (*length == 0 && close != ZOIDFS_CLOSE)
	 return ZOIDFS_BUF_ERROR;

  xdr_header_size = xdr_sizeof((xdrproc_t)xdr_int, &xdr_header_size);

  /* Dump any remaining output buffer */

  ret = lzf_dump_comp_buffer (stream, dest, output_length);

  if (ret == ZOIDFS_OUTPUT_FULL)
    return ret;
 
  do 
    {
      /* make some room for the header */
      stream->out_buf += xdr_header_size + 1;
      
      comp_ret = lzf_compress_block (stream, source, length, close);
      stream->out_buf -= xdr_header_size + 1;
      if (comp_ret == ZOIDFS_BUF_ERROR)
    	  break;
      lzf_compress_gen_header (stream, xdr_header_size);

      ret = lzf_dump_comp_buffer (stream, dest, output_length);
      if (ret == ZOIDFS_OUTPUT_FULL)
	break;
    }
  while (comp_ret != ZOIDFS_COMPRESSION_DONE);
  /* Create header for packet */

  if (comp_ret == ZOIDFS_COMPRESSION_DONE)
	 return ZOIDFS_STREAM_END;
  if ((*length) == 0  || (*output_length) == 0)
    return ZOIDFS_BUF_ERROR;
}
#define LZF_UNCOMPRESSED 5
#define LZF_COMPRESSED 6
int lzf_decompress_get_dataset_rem (lzf_state_var * state, void ** source,
				    size_t * source_len, void ** packet,
				    size_t * packet_len)
{
  XDR xdr;
  int ret;
  int header_rem = 0;
  char * xdr_buffer;
  size_t xdr_header_size;  
  xdr_header_size = xdr_sizeof((xdrproc_t)xdr_int, &xdr_header_size);
  
  /* if the header is not known yet, find it and make a copy */
  if (state->in_buf_len < (1 + xdr_header_size))
    {
      header_rem = (1 + xdr_header_size) + state->in_buf_len;
      memcpy (state->in_buf, (*source), header_rem);
      
      (*source_len) -= header_rem;
      state->in_buf_len += header_rem;
      (*source) += header_rem;
      state->in_buf +=  header_rem;
    }
  
  state->in_buf -= state->in_buf_len - 1;
  xdr_buffer = state->in_buf;
  xdrmem_create(&xdr,xdr_buffer,xdr_header_size,XDR_DECODE);   
  if (xdr_int (&xdr, packet_len) != 0)
    return -1;

  if (*source_len >= *packet_len - 
      (state->in_buf_len - xdr_header_size - 1))
    {
      state->in_buf += state->in_buf_len - 1;
      memcpy(state->in_buf, (*source), *packet_len - 
	     (state->in_buf_len - xdr_header_size - 1));
      state->in_buf -= state->in_buf_len;
      state->in_buf_len +=  *packet_len - 
	                    (state->in_buf_len - xdr_header_size - 1); 
    }
  else
    {
      state->in_buf += state->in_buf_len - 1;
      memcpy(state->in_buf, (*source), *source_len);
      state->in_buf_len += *source_len;
      state->in_buf += *source_len;
      return ZOIDFS_BUF_ERROR;
    }

  if  (((char*)state->in_buf)[0] == 'C')
    ret = LZF_COMPRESSED;
  else if (((char*)state->in_buf)[0] == 'U')
    ret = LZF_UNCOMPRESSED;
  else 
    return -1;

  state->in_buf += 1 + xdr_header_size;
  (*packet) = state->in_buf;
  
  return ret;
}

int lzf_decompress_get_dataset_full (lzf_state_var * state,
				     void ** source, size_t * source_len,
				     void ** packet, size_t * packet_len)
{
  XDR xdr;
  char * xdr_buffer;
  int ret = 0;
  char * source_char = (char *) (*source);
  size_t xdr_header_size;  

  if (*source_len < (1 + xdr_header_size))
    {
      memcpy(state->in_buf,source_char, (*source_len));
      state->in_buf_len = (*source_len);
      state->in_buf_pos = 0;
      source_char += (*source_len);
      (*source) = (void *) source_char;
      return ZOIDFS_BUF_ERROR;
    }

  if (state->in_buf_len == 0)
    {
      /* Parse the header charactor */
      if (source_char[0] == 'U')
	ret = LZF_UNCOMPRESSED;
      else if (source_char[0] == 'C')
	ret = LZF_COMPRESSED;
      else
	return -1;
      source_char++;
  
      /* Parse the xdr_header_string */

      xdr_buffer = source_char;
      xdrmem_create(&xdr,xdr_buffer,xdr_header_size,XDR_DECODE);   
      if (xdr_int (&xdr, packet_len) != 0)
	return -1;

      if (*source_len < *packet_len)
	{
	  source_char--;
	  memcpy(state->in_buf, source_char, *source_len);
	  state->in_buf += *source_len;
	  state->in_buf_len = *source_len;
	  state->in_buf_pos = *source_len;
	  source_char += (*source_len);
	  *source_len = 0;
	  return ZOIDFS_BUF_ERROR;
	}
      else
	{
	  source_char += xdr_header_size;
	  (*packet) = (void *) source_char;
	  source_char += *packet_len;
	}
    }
  return ret;
}

int lzf_decompress_get_dataset (lzf_state_var * state,
				void ** source, size_t * source_len,
				void ** packet, size_t * packet_len)
{
  int ret;
  if ((*source_len) == 0)
    return ZOIDFS_BUF_ERROR;
  if (state->in_buf_len > 0)
    ret = lzf_decompress_get_dataset_rem (state, source, source_len,
					  packet, packet_len);
  else
    ret = lzf_decompress_get_dataset_full (state, source, source_len,
					   packet, packet_len);
  return ret;
}
inline void lzf_decompress_reset_state (lzf_state_var * state)
{
  state->out_buf -= state->out_buf_pos;
  state->out_buf_pos = 0;
  state->out_buf_len = 0;
}

int lzf_decompress_write_block (lzf_state_var * state, void ** dest,
				size_t * dest_len)
{
  size_t rem_out = 0;
  if (state->out_buf_len > 0)
    {
      rem_out = (state->out_buf_len - state->out_buf_pos);
      if ((*dest_len) >= rem_out)
	{
	  memcpy((*dest),state->out_buf,rem_out);
	  (*dest) += rem_out;
	  (*dest_len) -= rem_out;
	  lzf_decompress_reset_state (state);
	  return ZOIDFS_CONT;
	}
      else 
	{
	  memcpy((*dest), state->out_buf, (*dest_len));
	  state->out_buf += (*dest_len);
	  state->out_buf_pos += (*dest_len);
	  state->out_buf_len -= (*dest_len);
	  (*dest_len) = 0;
	  return ZOIDFS_BUF_FULL;
	}
    }
  return ZOIDFS_CONT;
}

int lzf_decompress_block (lzf_state_var * state, void * block,
			  size_t block_len, int type)
{
  int ret = 0;
  if (LZF_COMPRESSED == type)
    {
      ret = lzf_decompress(block, block_len, state->out_buf, state->out_buf_size);
      state->out_buf_len = ret;
    }
  else
    {
      memcpy(state->out_buf, block, block_len);
      state->out_buf_len = block_len;
    }
  return ret;
}


int lzf_decompress_hook (void * state_var, void ** source,
			 size_t * source_len, void ** dest,
			 size_t * dest_len, int close)
{
  char * decomp_block;
  size_t * decomp_size = 0;
  int ret, end;
  lzf_state_var * state = (lzf_state_var *)state_var;
  lzf_dump_comp_buffer (state, dest, dest_len);
  do 
    {
      ret = lzf_decompress_get_dataset (state, source, source_len, decomp_block, decomp_size);
      if (ret == ZOIDFS_BUF_ERROR)
	break;
      lzf_decompress_block (state, decomp_block, decomp_size, ret);
      if (state->out_buf_len < LZF_BUFF_SIZE)
	end = ZOIDFS_DECOMPRESSION_DONE;
      ret = lzf_dump_comp_buffer (state, dest, dest_len);
      state->in_buf -= state->in_buf_len;
      state->in_buf_len = 0;
      if (ret == ZOIDFS_OUTPUT_FULL)
	break;

    } while (end != ZOIDFS_DECOMPRESSION_DONE);
  return end;
}
 
/***********************
 Decompression Function

void * lzf_decompress_data (void * compressed_data, int len)
{
    XDR  header_data;
    int header_size = 0;    
    header_size = xdr_sizeof((xdrproc_t)xdr_int, &header_size);
    char xdr_buffer[header_size]; 
    
    char * data = malloc(1280000);
    int data_pos = 0;
    int chunk_size = 0;
    int compressed_pos = 0;
    int count =0;
    int x,ret;
    for(;;)
    {
        if (LZF_DEBUG)
        {
            fprintf(stderr,"Decompressed run : %i\n\tReturn: %i\n",count,ret);
            fprintf(stderr,"\tOutput Buffer:\n\t\tMemory_loc: %p\n\t\tPosition: %i\n",
                            data, data_pos);
            fprintf(stderr,"\tCompressed Buffer:\n\t\tMemory_loc: %p\n\t\tFirst Byte: %c\n",
                            compressed_data,((char *)compressed_data)[0]);
        }
        count++;
        /* if the data is uncompressed *
        if (data_pos == 1280000)
            break;
        if(((char *)compressed_data)[0] == 'U')
        {
            for(x = 0; x < header_size; x++)
                xdr_buffer[x] = ((char *)compressed_data)[x+1];
            compressed_data += header_size + 1;
            xdrmem_create(&header_data,xdr_buffer,header_size, XDR_DECODE); 
            xdr_int(&header_data,&chunk_size);
            memcpy(data,compressed_data,chunk_size);
            data += chunk_size;
            data_pos += chunk_size;
            compressed_data += chunk_size;
        }
        else if (((char *)compressed_data)[0] == 'C')
        {
            for(x = 0; x < header_size; x++)
                xdr_buffer[x] = ((char *)compressed_data)[x+1];
            compressed_data += header_size + 1;
            xdrmem_create(&header_data,xdr_buffer,header_size, XDR_DECODE); 
            xdr_int(&header_data,&chunk_size);     
            /* Compressed the data *
            ret = lzf_decompress (compressed_data, chunk_size, data, 1280000 - data_pos);
            compressed_data += chunk_size;
            data += ret;
            data_pos += ret;
        }
        else 
        {
            fprintf(stderr,"NOT A PROPER LZF COMPRESSION!\n");
            break;
        }
    }
    data -= data_pos;
    return data;
}
*/


