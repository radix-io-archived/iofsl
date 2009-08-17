#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"
#include "c-util/gencache.h"
#include "c-util/tools.h"

/* number of items to add to the cache */ 
#define ADD_COUNT 100

/* capacity of gencache */
#define CACHE_SIZE 13

static gencache_handle handle; 

/* we use an intptr_t as key and a string as value */

static int item_free (void * item)
{
   free ((char *) item); 
   return 1; 
}

static int key_compare (const void * item1, const void * item2)
{
   return (intptr_t) item1 == (intptr_t) item2; 
}

static int key_free (void * UNUSED(item))
{
   /* no need to free key */ 
   return 1;
}

static unsigned long key_hash (gencache_key_t key)
{
   return (unsigned long) key; 

   /*unsigned char init[sizeof(unsigned long)]; 
   const char * ptr = (const char *) key; 
   unsigned int pos = 0; 
   char c; 

   memset (&init[0], 0, sizeof(init)); 

   while ((c=*ptr++))
   {
      init[pos++] ^= c; 
      if (pos >= sizeof(unsigned long))
         pos = 0; 
   }
   return *(unsigned long *) (&init[0]);  */
}

static int init_suite ()
{
   gencache_init_t i; 
   i.max_cache_size=CACHE_SIZE;
   i.fn_item_free = item_free;
   i.fn_key_free = key_free;
   i.fn_key_compare = key_compare;
   i.fn_key_hash = key_hash; 

   handle = gencache_init (&i); 
   return 0; /* success */ 
}

static int done_suite ()
{
   gencache_done (handle); 
   return 0; /* success */ 
}

static void addItem (intptr_t i)
{
      char buf[255]; 
      sprintf (buf, "%i", (int) i); 
      char * str = strdup (buf); 
      gencache_key_add (handle, (gencache_key_t) i, 
            str, 0, 0); 
}

static void test1 ()
{
   /* add 100 items */ 
   intptr_t i; 
   for (i=0; i<ADD_COUNT; ++i)
   {
      addItem (i); 
   }
}

static void test2 ()
{
   /* Try looking up strings; we should have the last CACHE_SIZE strings */ 
   intptr_t i; 
   for (i=0; i<ADD_COUNT; ++i)
   {
      void * found = 0; 
      int ret = gencache_key_lookup (handle, (gencache_key_t) i, 
            &found, 0); 
      if (i < (ADD_COUNT - CACHE_SIZE))
      {
         CU_ASSERT_FALSE (ret); 
      }
      else
      {
         CU_ASSERT_TRUE (ret); 
         CU_ASSERT_EQUAL (i, atoi (found)); 
      }
   }
}

static void checkItem (intptr_t key, const char * data)
{
   CU_ASSERT_EQUAL ((int) key, atoi (data)); 
}

static void test3 ()
{
   unsigned int i; 
   /* add random items, and lookup random even items */
   for (i=0; i<ADD_COUNT; ++i)
   {
      void * data;
      intptr_t key; 
      key = ((random () % ADD_COUNT) >> 1) << 1;  
      if (gencache_key_lookup_refresh (handle, (gencache_key_t) key, &data, 0))
         checkItem (key, data); 
      
      key = (random () % ADD_COUNT); 
      if (!gencache_key_lookup (handle, (gencache_key_t) key, &data, 0))
         addItem (key); 
   }
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
   if ((NULL == CU_add_test(pSuite, "Adding entries", test1)) ||
       (NULL == CU_add_test(pSuite, "Checking ordered expire", test2)) ||
       (NULL == CU_add_test(pSuite, "Lookup and add mix", test3)))
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
