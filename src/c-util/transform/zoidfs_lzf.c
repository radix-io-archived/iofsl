#include "zoidfs_transform.h"

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
    (*tmp).buf = malloc(LZF_BUFF_SIZE);
    (*tmp).cur_position = 0;

    (*library) = tmp;     
    return 0;
}

int lzf_compress_hook (void * stream, void ** source, size_t * length, void ** dest,
                        size_t * output_length, int close)
{
    return lzf_compress_data((lzf_state_var *)stream, source, length, dest, output_length, close);
}
int lzf_compress_data (lzf_state_var * stream, void ** source, size_t * length, void ** dest,
                       size_t * output_length, int close)
{
    int count = 0;
    /* Sets up the xdr encoding */   
    XDR  header_data;
    int header_size = 0;    
    header_size = xdr_sizeof((xdrproc_t)xdr_int, &header_size);
    char xdr_buffer[header_size]; 
    
    /* Stores the input data */
    void * data = (*source);
    int data_pos = 0;

    /* Compressed buffer that will be passed into lzf_compress_data
       to store precompressed data +  headers */
    void * compressed = (*dest);
    int comp_pos = 0;
    int comp_buf_size = *output_length;

    /* internal state buffer to store possible overflows */   
    void * buffer = malloc(LZF_BUFF_SIZE + header_size + 1);
    int buf_pos = 0;
    int buf_left = (*stream).cur_position;
    int x;
    int ret = 0;

    /* Write whats left hanging around in the buffers */
    if (buf_left > 0 && (comp_buf_size - comp_pos) > buf_left)
    {
        memcpy (compressed, buffer, buf_left);
        compressed += buf_left;
        comp_pos += buf_left;
        buffer -= buf_pos;
        buf_pos = 0;
        buf_left = 0;
    }
    /* if for some reason we cant write this data out return */
    else if (buf_left > 0)
    {
        return ZOIDFS_BUF_ERROR;
    }
    /* if there is no input, check to see if the EOS flag was set
        if so exit. (No buffer left at this point) */
    if ((*length) == 0)
        if ((close) == ZOIDFS_CLOSE)
            return ZOIDFS_STREAM_END;
        else
            return ZOIDFS_BUF_ERROR;

    /* Compress the data until the buffer is full */
    while (comp_pos < comp_buf_size)
    {
       /* Mark the buffer as compressed */
       ((char *)buffer)[0] = 'C';

       /* move the buffer forward to make room for xdr_int */
       buffer += header_size + 1;  
       
       /* Compressed the data */
       ret = lzf_compress (data, LZF_BUFF_SIZE, buffer, LZF_BUFF_SIZE);

       /* If the compression fails, just put raw data there */
       if (ret == 0)
       {
            /* Moveing the buffer back to set uncompressed flag */
            buffer -= header_size + 1;  
            ((char *)buffer)[0] = 'U';
            
            /* Advancing the buffer back */
            buffer += header_size + 1;                  
            memcpy(buffer, data, LZF_BUFF_SIZE);
            ret = LZF_BUFF_SIZE;                
       }
       buffer -= header_size + 1;   
       data_pos += LZF_BUFF_SIZE;

       /* If at end of data reset pointer */
       if (data_pos > (*length))
           data -= data_pos - LZF_BUFF_SIZE;
       else
           data += LZF_BUFF_SIZE;

       /*Generate header */
       xdrmem_create(&header_data,xdr_buffer,header_size,XDR_ENCODE); 
       xdr_int(&header_data,&ret);
       
       /* Copy the header data */
       for (x = 0; x < header_size; x++)
       {
            ((char *)buffer)[x + 1] = xdr_buffer[x];
       }

       xdr_destroy(&header_data);
       buf_left = ret + header_size + 1;
       /* If there is enough buffer to write out the compressed 
            data fully */
       if (buf_left < (comp_buf_size - comp_pos))
       {
            memcpy (compressed,buffer,buf_left);
            compressed += buf_left;
            comp_pos += buf_left;
            buf_left = 0;
            buf_pos = 0;
       }
       /* else: write out what you can and signal to caller that you
            need more room */
       else 
       {
            buf_pos = (comp_buf_size - comp_pos);
            memcpy(compressed,buffer,buf_pos);
            compressed += buf_pos;
            buf_left = buf_left - buf_pos;
            buffer += buf_pos;
            comp_pos += buf_pos;
            break;
        }
        /* if the data_position is greater then or equal to the length
            of the output string exit the compression loop */
        if (data_pos >= (*length))
            break;
    }
    buf_left = (*stream).cur_position;

    (*output_length) = (*output_length) - comp_pos;
    if (data_pos > (*length))
        (*length) = 0;
    else
        (*length) = (*length) - data_pos;
    (*dest) = compressed;
    (*source) = data;
    if ((*length) == 0  || (*output_length) == 0)
        return ZOIDFS_BUF_ERROR;
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


