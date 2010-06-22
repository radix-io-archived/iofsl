#include "zoidfs/client/zoidfs_transform.h"
#include "CUnit/Basic.h"
#include <stdio.h>
#include <stdlib.h>

static void ** test_data;
static size_t num_test_data = 10;
static size_t data_size = 40000000;
int init_transform_test(void)
{
    int x, y;
    srand((unsigned)time(NULL));
    test_data = malloc(num_test_data * sizeof(char *));
    for ( x = 0; x < num_test_data; x++)
    {
        test_data[x] = malloc(x * data_size * sizeof(char));
        for ( y = 0; y < x * data_size; y++)
        {
            ((char **)test_data)[x][y] = (char)(rand() % 256);
        }
    }
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
    int msg_length = data_size;             /* length of the input buffer               */
    void * output = malloc(50000);          /* buffer that stores the compressed output */
    int output_length = 150;                /* size of the output buffer to create      */
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
            retval = zlib_compress (compression, test_data, &msg_length,  
                           &output, &output_length, 
                            0);
        }
        else
        {
            msg_length = 0;
            retval = zlib_compress (compression, test_data, &msg_length,  
                               &output, &output_length, 
                               1);
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
