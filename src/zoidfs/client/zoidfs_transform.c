#include "zoidfs_transform.h"

zoidfs_write_compress init_compression (char * type, void ** input_buffers, 
                                        size_t input_buf_len[], size_t num_input_bufs,
                                        size_t pipeline)
{
    int x;
    zoidfs_write_compress comp;
    compress_init (type,&comp.comp_struct);
    comp.input_buffers = input_buffers;
    comp.input_buf_len = input_buf_len;
    comp.input_num_buffers = num_input_bufs;
    comp.output_buf = malloc(pipeline);
    comp.input_remaining = malloc(sizeof(size_t) * num_input_bufs);
    for (x = 0; x < num_input_bufs; x++)
    {
        comp.input_remaining[x] = comp.input_buf_len[x];
    }
    comp.pipeline_size = pipeline;
    comp.current_buf = 0;
    return comp;
}

int zoidfs_compress  (zoidfs_write_compress compression)
{
    int i,ret;
    int close = 0;
    size_t remaining = 0; 
    size_t total_buf_size = 0;   
    size_t output_size = compression.pipeline_size;
    for (i = compression.current_buf; i < compression.input_buf_len; i++)
    {
        ret = compress_buffer (compression);
        /* If the return indicates the buffer is not full, continue on with
           next buffer */
        if (ret == ZOIDFS_CONT)
            compression.current_buf++;
        /* If the compression is complete, reset the output buffer pointer and
           return */
        else if (ret == ZOIDFS_COMPRESSION_DONE)
        {
            compression.output_buf -= compression.output_buf_size;
            return ZOIDFS_COMPRESSION_DONE;
        }
        /* If there is no more room in the buffer return to the caller */
        else
        {
            compression.output_buf -= compression.output_buf_size;
            return ZOIDFS_CONT;
        }
    }
}
int compress_buffer (zoidfs_write_compress compression)
{
    int ret;
    int buf_num = compression.current_buf;
    int close = 0;
    size_t output_size = compression.pipeline_size - compression.output_buf_size;
    size_t buf_left = compression.input_remaining[buf_num];
    do {
        /* compress the stream  */
        ret = zlib_compress(compression.input_buffers[buf_num], &buf_left,
                            &compression.output_buf, &output_size, 
                            compression.comp_struct, close);  
        /* Add to the current size of the output buffer by the amount outputted by compress */  
        compression.output_buf_size += output_size;
        /* Update the remaining amount of data in this buffer */
        compression.input_remaining[buf_num] = compression.input_buf_len[buf_num] - buf_left;
        /* Move the output buffer pointer up */
        compression.output_buf += output_size;
        /* If the output buffer is full return */
        if (compression.output_buf_size == compression.pipeline_size)
        {
            return ZOIDFS_BUF_FULL;
        }
        /* If there is no more input flush so we can get the position in the output stream 
           for metadata purpose's */
        if (compression.input_remaining[buf_num] == 0)
        {
            if (buf_num == compression.input_num_buffers)
                close = Z_FINISH;
            else if (close != Z_FULL_FLUSH)
                close = Z_FULL_FLUSH;
            /* Possibly change this to Z_OK if deflace and be determined to always return
               the full internal buffer on compress call with Z_FULL_FLUSH settings */
            else if (ret ==  Z_BUF_ERROR)
                return ZOIDFS_CONT;
            else if (ret == Z_STREAM_END)
                return ZOIDFS_COMPRESSION_DONE;
        }
    } while (ret == Z_OK);
    return ret;
} 

int compress_init (char * type, void ** library)
{
    int ret; 
    void ** ret_value;
    
    if (strcmp(type,"zlib") == 0)
    {
        /* What level of compression to user (static for now) */
        int level = 0;

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

void test_compression (void)
{
    int z;
    /* generate some testing data */
    void * test_data = malloc(5000);
    unsigned char * set_data = test_data;
    for ( z = 0; z < 5000; z++)
    {
         set_data[z] = (unsigned char)z;
    }
    
    /* Simple compression test case */
    size_t cur_allocation = 0;
    void * storage_buffer = malloc(10000);
    int data_size  = 5000;
    int msg_length = data_size;          /* length of the input buffer               */
    void * output = malloc(50000);                          /* buffer that stores the compressed output */
    int output_length = 150;             /* size of the output buffer to create      */
    void * compression;                     /* stores the compression struct            */
    compress_init ("zlib", &compression);   /* Initializes the compression stream       */
    int loc = 0;
    int retval;

    void * decompress;
    decompress_init ("zlib", &decompress);

    do                                      /* Compress the data                        */
    { 
        output_length = 150;
        if ((int)msg_length > 0)
        {
            retval = zlib_compress (test_data, &msg_length,  
                           &output, &output_length, 
                           compression, 0);
        }
        else
        {
            msg_length = 0;
            retval = zlib_compress (test_data, &msg_length,  
                               &output, &output_length, 
                               compression, 1);
            msg_length = 0;
        }
        for (z = cur_allocation; z < cur_allocation + output_length; z++)
        {
            ((unsigned char *) storage_buffer)[z] = ((unsigned char *)output)[z - cur_allocation];
        }
        cur_allocation += output_length;
        data_size = data_size - msg_length; 
        msg_length = data_size; 
    } while (retval != Z_STREAM_END);
    FILE * test = fopen("testfile.x","w");
    fwrite(storage_buffer,1,cur_allocation, test);
    fclose(test);
    printf("current allocation %i\n", cur_allocation);
    size_t compressed_len = cur_allocation;
    msg_length = compressed_len;
    cur_allocation = 0;
    retval = -111;
    size_t total = 0;
    output_length = 5000;
    printf("Locations\nstorage buf: %i\nmsg_length: %i\noutput length: %i\n", 
            storage_buffer, msg_length, output_length);
    retval = zlib_decompress ( storage_buffer, &msg_length,  
                   &output, &output_length, 
                   decompress, 1);
    int x = 0;
    for (x = 0; x < 5000; x ++)
    {
        assert ((unsigned char) x == ((unsigned char *) output)[x]);
    }
}

/* compress or decompress from stdin to stdout */
int main(int argc, char **argv)
{   
    test_compression ();
}
