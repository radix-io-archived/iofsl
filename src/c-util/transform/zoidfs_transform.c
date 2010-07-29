#include "zoidfs_transform.h"
#include <string.h>
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
	if (close != 0)
	{
	  return ZOIDFS_STREAM_END;
	}
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

int zoidfs_transform_memcpy (void * stream, void ** source, size_t * length, void ** dest,
			     size_t * output_length, int close)
{
  /* Memory copy transform. Copy's input memory into output memory */
  if (*output_length > *length)
    {
      memcpy ((*dest), (*source), (*length));
      (*source) += *length;
      *output_length =  *output_length - *length;
      (*dest) += *length;
      *length = 0;
      if (close != 0)
	{
	  return ZOIDFS_STREAM_END;
	}
    }
  else
    {
      memcpy ((*dest), (*source), (*output_length));
      (*source) += *output_length;
      *length = *length - *output_length;        
      (*dest) += *output_length;
      *output_length = 0;           
    } 
  return ZOIDFS_BUF_ERROR;
}
int zoidfs_transform_init (char * type_str, zoidfs_write_compress * comp)
{
  char * tmp_type = malloc(sizeof(char) * strlen(type_str) + 1);
  char * main_type;
  int x;
  int level = -1;
  char * option;
  char * type;

  /* Get token for the name of the transform type */
  strcpy (tmp_type, type_str);
  main_type = strtok(tmp_type," :");
  type = malloc((strlen(main_type) + 1) * sizeof(char));

  /* Make the type lower case (for strcmp reasons) */
  for (x = 0; x < strlen(main_type); x++)
    type[x] = tolower(main_type[x]);
  type[strlen(main_type)] = '\000';

  /* Setup the transform struct */
  (*comp).type = type;
  (*comp).buf_position = 0;
  (*comp).intern_buf = NULL;

  if (strcmp("zlib",type) == 0)
    { 
#ifdef HAVE_ZLIB
      /* Grab the level from the input string */
      option = strtok(NULL," :");
      if (option != NULL)
	level = atoi(option);

      /* set up the compression struct and function pointer for 
	 transform */
      compress_init ("zlib",level,&(*comp).compression_struct);
      (*comp).transform = &zlib_compress_hook;
#else
      fprintf(stderr,"Error! Zlib library is not availible!\n");
      return -1;
#endif
    }
  else if (strcmp("bzip",type) == 0)
    {
#ifdef HAVE_BZLIB 
      /* Get the level from the options */
      option = strtok(NULL,":");
      if (option != NULL)
	level = atoi(option);
      
      /* set up the compression struct */
      bzip_compress_init ("bzip", 9, 0, 30, &(*comp).compression_struct);
      (*comp).transform = &bzip_compress_hook; 
#else 
      fprintf(stderr,"ERROR! bzip library is not availible!\n");
      return -1;
#endif 
    }
  else if (strcmp("lzf",type) == 0)
    {
#ifdef HAVE_LZF
      lzf_compress_init (&(*comp).compression_struct,(*comp).total_in);
      (*comp).transform = &lzf_compress_hook;
#else
      fprintf(stderr,"ERROR! LZF library is not availible!\n");
      return -1;
#endif
    }
  else if (strcmp("passthrough",type) == 0)
    {
      (*comp).transform = &passthrough;                    
    }
  else if (strcmp("memcpy", type) == 0)
    {
      (*comp).transform = &zoidfs_transform_memcpy;
    }
  else
    {
      free(type);
      assert("Transform not recognized" == NULL);
    }

  free(tmp_type);
  return 0;
}

void zoidfs_transform_destroy (zoidfs_write_compress * comp)
{
    if (strcmp((*comp).type,"ZLIB:") == 0 || strcmp((*comp).type,"BZIP:") == 0)
    {
        free((*comp).compression_struct);        
    }
} 

void zoidfs_transform_change_transform (char * type, zoidfs_write_compress * comp)
{
  zoidfs_transform_destroy (comp);
  zoidfs_transform_init (type,comp);
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

int zoidfs_transform_write_request (zoidfs_write_compress * transform,
				    zoidfs_write_vars * write_buffs,
				    size_t max_buff_size,
				    size_t num_of_buffs_to_fill,
				    void *** buffer,
				    size_t ** buffer_sizes,
				    size_t * buf_count,
				    size_t * total_len,
				    int * close
				    )

{
  /* flag to say to exit when a certain size is reached */

  int exit_when_size_reached;
  
  int x = 0,y = 0, rem_output_bufs, ret;
  (*buf_count) = 1;
  
  /* if the num of buffers to fill is not set, fill them all */
  
  if (num_of_buffs_to_fill == 0)
    {
      rem_output_bufs = write_buffs->mem_count - 1;
      exit_when_size_reached = 1;
    }
  else
    {
      rem_output_bufs = num_of_buffs_to_fill - 1;
      exit_when_size_reached = 0;

    }

  /* Stores the length of the remaining input buffer (pre-transformed) */
  size_t input_buf_left = write_buffs->mem_sizes[0];

  /* Stores the length of the remaining output buffer */
  size_t output_buf_left = max_buff_size;

  void * input_buffer;
  void * output_buffer;

  /* Malloc's memory for the first buffer */
  if (transform->type != "passthrough")
    (*buffer)[0] = malloc(max_buff_size * sizeof(char));
  
  /* Loop until transform compleated or out of buffer */
  do
    {
      /* Does the transform */

      ret = zoidfs_transform( transform, &(*write_buffs).mem_starts[x],  
			      &input_buf_left, &(*buffer)[y], &output_buf_left,
			      *close);

      /* If the output buffer is full OR compression is compleated */

      if (output_buf_left == 0 || ret == ZOIDFS_COMPRESSION_DONE)
	{
	  (*buffer)[y] -= max_buff_size - output_buf_left;
	  write_buffs->mem_sizes[x] = input_buf_left;
	  (*buffer_sizes)[y] = max_buff_size - output_buf_left;
	  (*total_len) += (*buffer_sizes)[y];

	  if ((*total_len) == max_buff_size && exit_when_size_reached == 1)
	    {
	       return ret;
	    }
	  if (rem_output_bufs > 0 && ret != ZOIDFS_COMPRESSION_DONE)
	    {
	      y++;
	      if (transform->type != "passthrough")
		(*buffer)[y] = malloc(max_buff_size * sizeof(char));
	      output_buf_left = max_buff_size;
	      rem_output_bufs -= 1;
	      (*buf_count) += 1;
	    }
	  else
	    {
      	      return ret;
	    }
	}

      /* if there was an error in the transform that is not related to the 
	 buffer return to caller with the error code */

      if (ret == ZOIDFS_TRANSFORM_ERROR)
	return ret;

      /* if there is no input buffer left ether grab the next buffer or close 
	 the stream */
      if (input_buf_left == 0)
	{
	  if ( x < (write_buffs->mem_count) - 1)
	    {
	      write_buffs->mem_sizes[x] = 0;
	      x++;
	      input_buf_left = write_buffs->mem_sizes[x];
	    }
	  else
	    {
	      (*close) = ZOIDFS_CLOSE;
	    }
	}
    } while (ret != ZOIDFS_COMPRESSION_DONE);

  return ret;  
}

int zoidfs_transform_decompress_init (char * type_str, zoidfs_decompress * comp)
{
  char * tmp_type = malloc(sizeof(char) * strlen(type_str) + 1);
  char * main_type;
  char * type;
  int x;

  /* Get token for the name of the transform type */
  strcpy (tmp_type, type_str);
  main_type = strtok(tmp_type," :");
  type = malloc((strlen(main_type) + 1) * sizeof(char));

  /* Make the type lower case (for strcmp reasons) */
  for (x = 0; x < strlen(main_type); x++)
    type[x] = tolower(main_type[x]);
  type[strlen(main_type)] = '\000';

  /* Setup the transform struct */
  comp->type = type; 

  if (strcmp("zlib",type) == 0)
    { 
#ifdef HAVE_ZLIB
      zlib_decompress_init (&(comp->transform));
      comp->decompress = &zlib_decompress;
#else
      fprintf(stderr,"Error! Zlib library is not availible!\n");
      return -1;
#endif
    }
  
  else if (strcmp("bzip",type) == 0)
    {
#ifdef HAVE_BZLIB 
      bzip_decompress_init (&(comp->transform));
      comp->decompress = &bzip_decompress; 
#else 
      fprintf(stderr,"ERROR! bzip library is not availible!\n");
      return -1;
#endif 
    }/*
  else if (strcmp("lzf",type) == 0)
    {
#ifdef HAVE_LZF
      lzf_decompress_init (&(comp->compression_struct));
      (*comp).transform = &lzf_compress_hook;
#else
      fprintf(stderr,"ERROR! LZF library is not availible!\n");
      return -1;
#endif
    }
  else if (strcmp("passthrough",type) == 0)
    {
      (*comp).transform = &passthrough;                    
    }
  else if (strcmp("memcpy", type) == 0)
    {
      (*comp).transform = &zoidfs_transform_memcpy;
    }
  else
    {
      free(type);
      assert("Transform not recognized" == NULL);
      }*/

  free(tmp_type);
  return 0;


}


int zoidfs_transform_decompress ( zoidfs_decompress * transform, 
				  void ** in_buf, size_t * in_size,
				  void *** out_buf, size_t ** out_size,
				  size_t * outputs_filled, size_t mem_count, int close)
{
  int x = 0;
  int ret = 0;
  (*outputs_filled) = 0;

  do
    {
      size_t tmp_store = (*out_size)[x];
      /*fprintf(stderr, "\nZoidfs_transform_decompress:\n\t"
	      "transform: %p\n\tin_buf: %p\n\tin_size: %i\n\t"
	      "out_buf: %p\n\tout_size: %i\r\noutputs_filled: %i\n\t"
	      "mem_count: %i\n\tclose: %i\n", transform->transform,
	      *in_buf,*in_size,(*out_buf)[x],(*out_size)[x],*outputs_filled,
	      mem_count, close);*/
      ret = transform->decompress(transform->transform, 
				  in_buf, in_size, 
				  &(*out_buf)[x], 
				  &(*out_size)[x], 0);

      /*fprintf(stderr, "\nZoidfs_transform_decompress:\n\t"
	      "transform: %p\n\tin_buf: %p\n\tin_size: %i\n\t"
	      "out_buf: %p\n\tout_size: %i\r\noutputs_filled: %i\n\t"
	      "mem_count: %i\n\tclose: %i\n", transform->transform,
	      *in_buf,*in_size,(*out_buf)[x],(*out_size)[x],*outputs_filled,
	      mem_count, close);*/
      if (ret == ZOIDFS_TRANSFORM_ERROR)
    	  return ret;
      if (*in_size == 0 && close != ZOIDFS_CLOSE)
    	  return ZOIDFS_BUF_ERROR;
      if (ret == ZOIDFS_STREAM_END)
    	  return ret;
      if ((*out_size)[x] == 0)
	{
	  x++;
	  (*outputs_filled) ++;
	}
    } while (x != mem_count);
  return ZOIDFS_OUTPUT_FULL;
}
int zoidfs_transform_read_request (zoidfs_decompress * transform,
				   zoidfs_read_vars * read_buffs,
				   size_t * outputs_filled,
				   int close
				   )
{
  int x, ret = 0;
  (*outputs_filled) = 0;
  for (x = 0; x < read_buffs->read_mem_count; x++)
    {

      if (read_buffs->read_buf_size[x] > 0)
	{
	  ret = zoidfs_transform_decompress (transform,
					     &read_buffs->read_buf[x],
					     &read_buffs->read_buf_size[x],
					     &read_buffs->output_buf,
					     &read_buffs->output_sizes,
					     &outputs_filled,
					     read_buffs->output_mem_count, 0);

	}
      if (x + 1 == read_buffs->read_mem_count &&
	  ret != ZOIDFS_STREAM_END && close != ZOIDFS_CLOSE)
	ret = zoidfs_transform_decompress (transform,
					   &read_buffs->read_buf[x],
					   &read_buffs->read_buf_size[x],
					   &read_buffs->output_buf,
					   &read_buffs->output_sizes,
					   &outputs_filled,
					   read_buffs->output_mem_count, close);
      if (ret == ZOIDFS_OUTPUT_FULL || ret == ZOIDFS_STREAM_END ||
	  ret == ZOIDFS_TRANSFORM_ERROR)
    	  return ret;
    }
}
