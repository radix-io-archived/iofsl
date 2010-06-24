#include "c-util/transform/zoidfs_transform.h"
#include "CUnit/Basic.h"
#include <stdio.h>
#include <stdlib.h>

static void ** test_data;

static size_t num_test_data = 10;
static size_t data_size = 400000;

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
            ((char **)test_data)[x][y] = (char)'a'+x;
        }
    }
    return 0;
}
int clean_transform_test(void)
{
    int x;
    for (x = 0; x < num_test_data; x++)
        free(test_data[x]);
    return 0;
}
void decompress_and_compare (void * output, size_t output_total, size_t output_size)
{
    #ifdef HAVE_ZLIB
    size_t compressed_len = output_total;
    size_t expected = output_size;
    void * tmp = malloc(output_size);
    void * decompress;    
    int x , y, loc = 0;
    decompress_init ("zlib", &decompress);
    int retval = zlib_decompress (output, &compressed_len,  
                                  &tmp, &output_size, 
                                  decompress, 1);    
    CU_ASSERT(retval == Z_STREAM_END);
    for ( x = 0; x < num_test_data; x++)
    {
        for ( y = 0; y < x * data_size; y++)
        {
            if (((unsigned char *)tmp)[loc] != ((unsigned char **)test_data)[x][y])
            {
                printf("Im wrong at loc: %i\n",loc);
                assert (((unsigned char *)tmp)[loc] == ((unsigned char **)test_data)[x][y]);
                //assert (((unsigned char *)tmp)[loc] == ((unsigned char **)test_data)[x][y]);
            }
            loc++;
        }
    }
    
    free(tmp);
    #else
    printf("Only zlib is supported for this test at this time\n");
    #endif
}
void test_zoidfs_client_send (void)
{
    int x = 0;
    void ** mem_starts = test_data;
    size_t mem_count = num_test_data;
    size_t mem_sizes[num_test_data];
    size_t total_len = 0;    
    for (x = 0; x < num_test_data; x++)
    {
        mem_sizes[x] = x * data_size;
        total_len += x * data_size;
    }
    size_t size = mem_sizes[0];

    int ret, close = ZOIDFS_CONT;
    zoidfs_write_compress zlib_struct;
    char * type = "nocompression";
    size_t buf_size = 16000, max_buf = 16000;
    zoidfs_transform_init (type, &zlib_struct);    
    x = 0;
    int y = 0;

    void ** buffer;
    size_t buf_count[mem_count];
    buffer = malloc(mem_count);
    total_len = 0;  
    do 
    {
       /* Transform the buffer */
       ret = zoidfs_transform (&zlib_struct, &mem_starts[x], &size, 
                               &buffer[y], &buf_size, close);
       printf("Buf size: %i, size: %i\n",buf_size,size);
       printf("Running\n");       
       /* send the data and reset the buffer positions */
       if (size == 0 && buf_size != 0)
       {
            buf_count[y] = mem_sizes[y];
            buffer[y] -= mem_sizes[y];
            total_len += mem_sizes[y];
            y++;
            x++;     
            printf("i moved\n");
       }   
       if (buf_size == 0 || ret == ZOIDFS_COMPRESSION_DONE)
       {
            buf_count[y] = max_buf - total_len;
    printf("%p %p\n",mem_starts[1],buffer[1]);
            printf("buf count = %i\n",buf_count[y]);
            buffer[y] -= buf_count[y];
    printf("%p %p\n",mem_starts[1],buffer[1]);
            total_len += buf_count[y];
            break;
       }
        
       /* if there is no more input in this buffer*/
       if ( size == 0)
       {   
            /* if there are additional buffers move on */
            if ( x < mem_count - 1 )
            {
                printf("X: %i\n",x);
                size = mem_sizes[x];
            }
        }
    } while(ret != ZOIDFS_COMPRESSION_DONE || buf_size == 0);
    printf("%p %p\n",test_data[0],buffer[0]);
    //assert(test_data[0] == buffer[0]);
    //assert(test_data[1] == buffer[1]);
    assert(buf_count[0] == 0);
    assert(buf_count[1] == 16000);
    printf("%p %p\n",(mem_starts[1]),buffer[1]);
    //test_data[1] += 16000;
    assert((mem_starts[1] - 16000) ==  buffer[1]);
}
void test_transform_zlib (void)
{
    #ifdef HAVE_ZLIB
    char * type = "zlib";
    void * output = malloc(200 * data_size);
    size_t output_size = 20 * data_size;
    size_t prev_size, size;
    int x,ret;
    size_t total_len = 0;    
    size_t output_total = 0;
    /* test to verify proper output size */
    zoidfs_write_compress zlib_struct;

    zoidfs_transform_init (type, &zlib_struct);    
    output_size = 250000;
    size = 0;
    
    /* Verify that zoidfs_transform returns ZOIDFS_CONT when no output is passed in */
    ret = zoidfs_transform (&zlib_struct, test_data[x], &size, &output, &output_size, ZOIDFS_CONT);
    CU_ASSERT(ret == ZOIDFS_CONT);

    /* This is slightly less then 250000 due to the fact that zlib adds some identifier bytes */
    CU_ASSERT(output_size == 249998);

    /* Check to make sure full 250000 buffers are coming back */
    for ( x = 1; x < num_test_data ; x++)
    {
       output_size = 250000;
       size = x * data_size;
       ret = zoidfs_transform (&zlib_struct, &test_data[x], &size, &output, &output_size, ZOIDFS_CONT);
        printf("Output_size = %i\n",output_size);       
        CU_ASSERT(output_size == 0);
       CU_ASSERT(ret == ZOIDFS_OUTPUT_FULL);
    } 

    /* Test to verify that the data being produced by zoidfs_transform is 
       valid */
    zoidfs_transform_init (type, &zlib_struct);
    output_size = 60 * data_size;
    prev_size = output_size; 

    for ( x = 1; x < num_test_data; x++)
    {
       /* Gets the size for this chunk of data */
       size = x * data_size;
       /* Adds the length of this uncompressed chunk to the total for
          all uncompressed data */
       total_len += size;
       /* Do the actual compression */ 
       ret = zoidfs_transform (&zlib_struct, &test_data[x], &size, 
                               &output, &output_size, ZOIDFS_CONT);
    }   
    /* Grab any data left in the buffers and close the stream */
    do 
    {
        ret = zoidfs_transform (&zlib_struct, &test_data[x], &size, &output, 
                                &output_size, ZOIDFS_CLOSE);

    } while (ret != ZOIDFS_COMPRESSION_DONE);  
    output -= (prev_size - output_size);
    /* Roll back the pointer and send to decompression and comparison testing
       function */
    output -= output_total;
    //decompress_and_compare ( output, (prev_size - output_size), total_len);
    
    zoidfs_transform_init (type, &zlib_struct);
    output_size = 150;    
    prev_size = output_size; 
    x = 1;
    size =  x * data_size;
    int close = ZOIDFS_CONT;
    size_t total_output_size = 0;
    do 
    {
       ret = zoidfs_transform (&zlib_struct, &test_data[x], &size, 
                               &output, &output_size, close);
       CU_ASSERT(ret != ZOIDFS_TRANSFORM_ERROR);
       if (ret == ZOIDFS_TRANSFORM_ERROR)
       {
         printf("Hello");
         break;
       }
       total_output_size += (150 - output_size);
       output_size = 150;
       if ( size == 0)
       {
            if ( x < num_test_data - 1 )
            {
                x++;
                size =  x * data_size;
            }
            else
            {
                close = ZOIDFS_CLOSE;
            }
        }
    } while(ret != ZOIDFS_COMPRESSION_DONE);
    output -= total_output_size;   
    decompress_and_compare ( output, total_output_size, total_len); 
    #else
    printf("Only zlib is supported for this test at this time\n");
    #endif
    
}

int main()
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Transform Test", init_transform_test, clean_transform_test);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   /* NOTE - ORDER IS IMPORTANT - MUST TEST fread() AFTER fprintf() */
   if ((NULL == CU_add_test(pSuite, "test of transform zlib",  test_transform_zlib)))
   {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* Run all tests using the CUnit Basic interface */
   CU_basic_set_mode(CU_BRM_VERBOSE);
   CU_basic_run_tests();
   CU_cleanup_registry();
   return CU_get_error();
}
