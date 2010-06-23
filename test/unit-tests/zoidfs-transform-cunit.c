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
            //printf("hello");
            if (((unsigned char *)tmp)[loc] != ((unsigned char **)test_data)[x][y])
                CU_ASSERT (((unsigned char *)tmp)[loc] == ((unsigned char **)test_data)[x][y])
            loc++;
        }
    }
    free(tmp);
}
    
void test_transform_zlib (void)
{
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
       CU_ASSERT(ret != -2);
       if (ret == -2)
         break;
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
   if ((NULL == CU_add_test(pSuite, "test of transform zlib", test_transform_zlib)))
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
