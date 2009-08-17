#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"
#include "src/zoidfs/dispatcher/posix2/dcache.h"
#include "c-util/tools.h"

/* number of items to add to the cache */ 
#define ADD_COUNT 100

/* capacity of dcache */
#define CACHE_SIZE 13

static dcache_handle handle; 


static int init_suite ()
{
   handle = dcache_create (CACHE_SIZE); 
   return 0; /* success */ 
}

static int done_suite ()
{
   dcache_destroy (handle); 
   return 0; /* success */ 
}

static void sethandle (zoidfs_handle_t * h, unsigned int i)
{
   memset (h, 0, sizeof(zoidfs_handle_t)); 
   /* store int into handle */ 
   memcpy (&h->data[5], &i, sizeof(i)); 
}

static int addItem (unsigned int i, Descriptor * d)
{
   int fd; 
   zoidfs_handle_t h; 
   int ret; 

   fd = open ("/dev/null", O_RDONLY);

   sethandle (&h, i); 
   ret = dcache_addfd (handle, &h, fd, d);
   CU_ASSERT_TRUE(ret); 
   return ret;
}

static void checkItem (const zoidfs_handle_t * UNUSED(h), const Descriptor * UNUSED(d))
{
   /*CU_ASSERT_EQUAL (*(unsigned int *) (&h->data[5]), d->fd ); */
}


static void test1 ()
{
   unsigned int i; 
   Descriptor d; 

   /* lookup missing items */ 
   for (i=0; i<100; ++i)
   {
      zoidfs_handle_t h; 
      sethandle (&h, ADD_COUNT + 1 + (random() % 1000)); 
      CU_ASSERT_FALSE(dcache_getfd (handle, &h, &d)); 
   }
}

static void test2 ()
{
   /* add 100 items */ 
   unsigned int  i; 
   int j; 
   Descriptor d; 

   for (i=0; i<ADD_COUNT; ++i)
   {
      if (addItem (i, &d))
      {
         dcache_releasefd (handle, &d); 
      }
   }

   /* Try looking up strings; we should have the last CACHE_SIZE strings */ 
   for (j=ADD_COUNT-1; j>=0; --j)
   {
      zoidfs_handle_t h; 
      sethandle (&h, j); 
      int ret = dcache_getfd (handle, &h, &d); 
      if (j < (ADD_COUNT - CACHE_SIZE))
      {
         CU_ASSERT_FALSE (ret); 
      }
      else
      {
         CU_ASSERT_TRUE (ret); 
         checkItem (&h, &d); 
         dcache_releasefd (handle, &d); 
      }
   }
}

static void test3 ()
{
   unsigned int i=0; 

   for (i=0; i<ADD_COUNT; ++i)
   {
      zoidfs_handle_t h; 
      unsigned int val = random () % ADD_COUNT; 
      sethandle (&h, val); 
      Descriptor d; 

      if (dcache_getfd (handle, &h, &d))
      {
         checkItem (&h, &d); 
         dcache_releasefd (handle, &d); 
      }
      else
      {
         if (addItem (val, &d))
         {
            dcache_releasefd (handle, &d); 
         }
      }
   }
}

int main (int UNUSED(argc), char ** UNUSED(args))
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("dcache", init_suite, done_suite);
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
