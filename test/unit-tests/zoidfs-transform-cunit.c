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

zoidfs_read_vars build_decompression (char ** data, size_t * len, size_t count)
{
  int y;
  zoidfs_read_vars x;
  x.read_buf = (void **) data;
  x.read_buf_size = len;
  x.read_mem_count = count;
  char ** out_data = malloc(sizeof(char *) * num_test_data);
  size_t * out_data_len = malloc(sizeof(size_t) * num_test_data);
  for (y = 0; y < num_test_data; y++)
    {
      out_data[y] = malloc(sizeof(char) * test_data_len[y]);
      out_data_len[y] = test_data_len[y];
    }
  x.output_buf = (void **)out_data;
  x.output_sizes = out_data_len;
  x.output_mem_count = num_test_data;
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
  size_t total_compressed_len = 0;
  size_t * local_mem_sizes = malloc(num_test_data * sizeof(size_t));
  void ** local_mem_starts = malloc(num_test_data * sizeof(void *));

  char ** compressed_mem_buffers = malloc(num_test_data * num_buffs * 20 *  sizeof(char *));
  size_t * compressed_mem_sizes = malloc(num_test_data * num_buffs * 20 * sizeof(size_t));
  size_t compressed_num_buffs = 0;
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
      if (transfer_type != "passthrough:")
	buffer[x] = malloc(sizeof(char) * out_buf_size);
      buffer_sizes[x] = out_buf_size;
    }


  do 
    {
      ret = zoidfs_transform_write_request (&transform, &var,
					    max_buf_size, num_of_buffs_to_fill,
					    &buffer, &buffer_sizes, &buf_count,
					    &total_len, &close);
      
      /* Copy compressed buffer pointers for decompression test later */
      for ( x = 0; x < buf_count; x++)
	{
	  compressed_mem_buffers[compressed_num_buffs] = buffer[x];
	  compressed_mem_sizes[compressed_num_buffs] = buffer_sizes[x];
	  total_compressed_len += buffer_sizes[x];
	  compressed_num_buffs++;
	  buffer_sizes[x] = out_buf_size;
	}
      total_len = 0;
      buf_count = num_buffs;

    } while (ret != ZOIDFS_COMPRESSION_DONE);

  /* Assert that the input data pointers have been advanced */
  for (x = 0; x < num_test_data; x++)
    {
      CU_ASSERT_EQUAL (var.mem_starts[x] - test_data_len[x], 
		       test_data[x]);  
    }

  /* Verify that the size of the compression is equal to what it originally was */
  size_t comp_size_test = 0;
  size_t * tmp_compressed_sizes = malloc(sizeof(size_t) * compressed_num_buffs);

  for (x = 0; x < compressed_num_buffs; x++)
  {
    comp_size_test += compressed_mem_sizes[x];
    tmp_compressed_sizes[x] = compressed_mem_sizes[x];
  }

  assert(comp_size_test == total_compressed_len);

  if (transfer_type != "passthrough:")
    {
      zoidfs_decompress decomp;
      zoidfs_transform_decompress_init (transfer_type, &decomp);
      zoidfs_read_vars read_buffs = build_decompression (compressed_mem_buffers
							 ,compressed_mem_sizes, 
							 compressed_num_buffs);

      ret = zoidfs_transform_read_request (&decomp, &read_buffs, &total_len, ZOIDFS_CLOSE);
      assert(ret != ZOIDFS_TRANSFORM_ERROR);
      for(x = 0; x <  compressed_num_buffs; x++)
      {
    	  compressed_mem_buffers[x] -= tmp_compressed_sizes[x];
      }
      int y;

      for(x = 0; x < read_buffs.output_mem_count; x++)
	{
	  read_buffs.output_buf[x] -= test_data_len[x];

	  for(y = 0; y < test_data_len[x]; y++)
	    {
	      if(((char **)read_buffs.output_buf)[x][y] != ((char **) test_data)[x][y])
		{
		  fprintf(stderr,"Error, Bytes do not match %i,%i, %c, %c",x,y,
			  ((char**)read_buffs.output_buf)[x][y],
			  ((char **) local_mem_starts)[x][y]);
		  assert(1 == 0);  
		}
	    }
	}


    }


  if (transfer_type != "passthrough:")
    {
      for(x = 0; x < compressed_num_buffs; x++)
	{
	  free(compressed_mem_buffers[x]);
	}    
    }
  //free(compressed_num_buffs);
  free(compressed_mem_sizes);
  free(buffer);
}

void test_zoidfs_transform_write_request (void)
{
  /* test compression 
  fprintf(stderr, "ZLIB TESTS\n");
  test_transform_write("ZLIB:", 15000000, 1, 1);
  test_transform_write("ZLIB:", 150000, 1, 1);
  test_transform_write("ZLIB:", 15000, 1, 1);
  test_transform_write("ZLIB:", 15000000, 5, 0);
  test_transform_write("ZLIB:", 1500, 5, 5);
  /*fprintf(stderr,"PASSTHROUGH TESTS\n"); 
  test_transform_write("passthrough:", 1500, 5 , 5);
  test_transform_write("passthrough:", 1500000, 1, 1);
  test_transform_write("passthrough:", 150000000, 1, 1); 
  fprintf(stderr,"BZIP TESTS\n");
  test_transform_write("bzip:", 1500, 1, 1);
  test_transform_write("bzip:", 15000, 1, 1);
  test_transform_write("bzip:", 1500000, 1, 1);*/
  
  test_transform_write("lzf:", 150000, 1, 1);

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
