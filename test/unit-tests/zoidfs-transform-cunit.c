#include "zoidfs/client/zoidfs_transform.h"
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
            ((char **)test_data)[x][y] = (char)'a' + x;
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
            CU_ASSERT (((unsigned char *)tmp)[loc] == ((unsigned char **)test_data)[x][y]);
            loc++;
        }
    }
}
    
void test_transform_zlib (void)
{
    void * output = malloc(200 * data_size);
    size_t output_size = 20 * data_size;
    int x,ret;
    size_t size;
    char * type = "zlib";

    /* test to verify proper output size */
    zoidfs_write_compress zlib_struct;

    zoidfs_transform_init (type, &zlib_struct);    
    output_size = 250000;
    size = 0;
    ret = zoidfs_transform (&zlib_struct, test_data[x], &size, &output, &output_size, 0);
    CU_ASSERT(ret == ZOIDFS_CONT);
    for ( x = 1; x < num_test_data; x++)
    {
       output_size = 250000;
       size = x * data_size;
       ret = zoidfs_transform (&zlib_struct, test_data[x], &size, &output, &output_size, 0);
       CU_ASSERT(output_size == 0);
    } 
    zoidfs_transform_destroy (&zlib_struct);    

    zoidfs_transform_init (type, &zlib_struct);
    output_size = 60 * data_size;
    size_t prev_size = 60 * data_size;
    size_t total_len =0;    
    size_t output_total = 0;
    for ( x = 1; x < num_test_data; x++)
    {
       size = x * data_size;
       total_len += size;
       ret = zoidfs_transform (&zlib_struct, test_data[x], &size, 
                               &output, &output_size, 0);
       output += (prev_size - output_size);
       output_total += (prev_size - output_size);
       prev_size = output_size;
    }   

    do 
    {
        ret = zoidfs_transform (&zlib_struct, test_data[x], &size, &output, 
                                &output_size, Z_FINISH);
        output += (prev_size - output_size);
        output_total +=(prev_size - output_size);
        prev_size = output_size;
    } while (ret != ZOIDFS_COMPRESSION_DONE);  

    output -= output_total;
    decompress_and_compare ( output, output_total, total_len);

    zoidfs_transform_destroy (&zlib_struct);
    zoidfs_transform_init (type, &zlib_struct);    
    
    
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
