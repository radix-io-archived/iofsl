#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"
#include "src/zoidfs/dispatcher/posix2/fcache.h"
#include "c-util/tools.h"

/* number of items to add to the cache */ 
#define ADD_COUNT 100

/* capacity of fcache */
#define CACHE_SIZE 13

static fcache_handle handle; 


static int init_suite ()
{
   handle = filename_create (CACHE_SIZE); 
   return 0; /* success */ 
}

static int done_suite ()
{
   filename_destroy (handle); 
   return 0; /* success */ 
}

static void sethandle (zoidfs_handle_t * h, unsigned int i)
{
   memset (h, 0, sizeof(zoidfs_handle_t)); 
   /* store int into handle */ 
   memcpy (&h->data[5], &i, sizeof(i)); 
}

static void addItem (unsigned int i)
{
   char buf[255]; 
   zoidfs_handle_t h; 

   sethandle (&h, i); 
   sprintf (buf, "%i", (int) i); 
   CU_ASSERT_TRUE(filename_add (handle, &h, buf)); 
}

static void test1 ()
{
   char buf[255]; 
   unsigned int i; 

   /* lookup missing items */ 
   for (i=0; i<100; ++i)
   {
      zoidfs_handle_t h; 
      sethandle (&h, ADD_COUNT + 1 + (random() % 1000)); 
      CU_ASSERT_FALSE(filename_lookup (handle, &h, buf, sizeof(buf))); 
   }
}

static void test2 ()
{
}

static void checkItem (const zoidfs_handle_t * h, const char * data)
{
   CU_ASSERT_EQUAL (*(unsigned int *) (&h->data[5]), (unsigned int) atoi (data)); 
}

static void test3 ()
{
}

int main (int UNUSED(argc), char ** UNUSED(args))
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("fcache", init_suite, done_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "Looking for missing entries", test1)) ||
       (NULL == CU_add_test(pSuite, "Checking ordered expire", test2)) ||
       (NULL == CU_add_test(pSuite, "Looking for missing entries (2)", test1)) ||
       (NULL == CU_add_test(pSuite, "Lookup and add mix", test3)) ||
       (NULL == CU_add_test(pSuite, "Looking for missing entries (3)", test1)))
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
