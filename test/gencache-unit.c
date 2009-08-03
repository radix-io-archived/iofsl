#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "c-util/gencache.h"
#include "c-util/tools.h"

static int init_suite ()
{
   return 0; /* success */ 
}

static int done_suite ()
{
   return 0; /* success */ 
}

static void test1 ()
{

}

int main (int UNUSED(argc), char ** UNUSED(args))
{
   CU_pSuite pSuite = NULL;

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("gencache", init_suite, done_suite);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if ((NULL == CU_add_test(pSuite, "test1()", test1)) ||
       (NULL == CU_add_test(pSuite, "test2fread()", test1)))
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
