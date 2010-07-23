#include "c-util/transform/zoidfs_transform.h"
#include "CUnit/Basic.h"
#include <stdio.h>
#include <stdlib.h>
#include "zoidfs/zoidfs.h"
#include "zoidfs/zoidfs-proto.h"
#include "iofwd_config.h"

static void ** test_data;
static size_t * test_data_len;
static size_t num_test_data = 10;
static size_t data_size = 400000;

int init_transform_test(void)
{
    int x, y;
    srand((unsigned)time(NULL));
    test_data = malloc(num_test_data * sizeof(char *));
    test_data_len = malloc ( sizeof(size_t) * num_test_data);
    for ( x = 0; x < num_test_data; x++)
    {
        test_data[x] = malloc(x * data_size * sizeof(char));
	test_data_len[x] = x * data_size;
        for ( y = 0; y < x * data_size; y++)
        {
            ((char **)test_data)[x][y] = (char)'a'+y;
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

typedef struct 
{
    zoidfs_write_compress * transform;
    zoidfs_write_vars * write_buffs;
    size_t max_buf_size;
    size_t num_of_buffs_to_fill;
    void ** buffer;
    size_t * buffer_sizes;
    size_t * buf_count;
    size_t * total_len;
} test_transform_data;

test_transform_data build_test_data (void)
{
  int y;
  test_transform_data x;
  zoidfs_write_vars * z = malloc(sizeof(zoidfs_write_vars));
  zoidfs_write_compress * comp = malloc(sizeof(zoidfs_write_compress));
  x.transform = comp;
    /* Stores a local copy of mem_starts and mem_sizes. This is used for 
       transform (which manipulates pointers). This ensures that the caller
       always gets the original pointer back */
    size_t * local_mem_sizes = malloc(num_test_data * sizeof(size_t));
    void ** local_mem_starts = malloc(num_test_data * sizeof(void *));
    for(y = 0; y < num_test_data; y++)
	{    
	    local_mem_sizes[y]  = (size_t)test_data_len[y];
	    local_mem_starts[y] = (void *)test_data[y];
	}

    (*z).mem_count   = num_test_data;
    (*z).mem_starts  = local_mem_starts;
    (*z).mem_sizes   = local_mem_sizes;
    (*z).file_count  = num_test_data;

    x.write_buffs = z;
    return x;
}
void test_transform_write (char * transfer_type, int out_buf_size, 
			   int num_buffs, int num_buffs_to_fill)
{
  int ret, x, y;
  size_t total_size = 0;
  zoidfs_write_vars var;

  zoidfs_write_compress transform;
  size_t max_buf_size = out_buf_size;
  size_t num_of_buffs_to_fill = num_buffs_to_fill;
  size_t buf_count = num_buffs;
  size_t total_len = 0;
  int close = 0;
  size_t * local_mem_sizes = malloc(num_test_data * sizeof(size_t));
  void ** local_mem_starts = malloc(num_test_data * sizeof(void *));
  FILE * test_file = fopen("/tmp/test.txt","w");
  char ** buffer = malloc(sizeof(char *) * num_test_data);
  size_t * buffer_sizes = malloc(sizeof(size_t) * num_test_data);
  /* Set up the local varibles for transform */

  for(y = 0; y < num_test_data; y++)
    {    
      local_mem_sizes[y]  = (size_t)test_data_len[y];
      local_mem_starts[y] = (void *)test_data[y];
      total_size += local_mem_sizes[y];
    }

  var.mem_count   = num_test_data;
  var.mem_starts  = local_mem_starts;
  var.mem_sizes   = local_mem_sizes;
  var.file_count  = num_test_data;

  zoidfs_transform_init (transfer_type, &transform);

  for (x = 0; x < num_test_data; x++)
    {
      if (transfer_type != "passthrough")
	buffer[x] = malloc(sizeof(char) * out_buf_size);
      buffer_sizes[x] = out_buf_size;
    }


  do 
    {
      ret = zoidfs_transform_write_request (&transform, &var,
					    max_buf_size, num_of_buffs_to_fill,
					    &buffer, &buffer_sizes, &buf_count,
					    &total_len, &close);

      for ( x = 0; x < buf_count; x++)
	{
	  fwrite(buffer[x],1,buffer_sizes[x],test_file);
	  buffer_sizes[x] = out_buf_size;
	}
      total_len = 0;
      buf_count = num_buffs;

    } while (ret != ZOIDFS_COMPRESSION_DONE);

  for (x = 0; x < num_test_data; x++)
    {
      CU_ASSERT_EQUAL (var.mem_starts[x] - test_data_len[x], 
		       test_data[x]);  
    }

  fclose(test_file);
  if (transfer_type != "passthrough")
    {
      for(x = 0; x < num_test_data; x++)
	{
	  free(buffer[x]);
	  //free(var.mem_starts[x]);
	}    
    }
}

void test_zoidfs_transform_write_request (void)
{
  /* test compression */
  test_transform_write("ZLIB:", 15000000, 1, 1);
  test_transform_write("ZLIB:", 150000, 1, 1);
  test_transform_write("ZLIB:", 15000, 1, 1);
  test_transform_write("ZLIB:", 15000000, 5, 5);
  test_transform_write("ZLIB:", 1500, 5, 5);
 
  test_transform_write("passthrough", 1500, 5 , 5);
  test_transform_write("passthrough", 1500000, 1, 1);
  test_transform_write("passthrough", 150000000, 1, 1);

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
   if ((NULL == CU_add_test(pSuite, "test of transform zlib",  test_zoidfs_transform_write_request)))
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
