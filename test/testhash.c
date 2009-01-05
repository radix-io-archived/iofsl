#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "iofwdhash.h"
#include "checks.h"

#define TABLESIZE 131
#define LOOPCOUNT 1000
#define LOOPCOUNT2 (LOOPCOUNT/10)

iofwdbool_t comparefunc (void * user, const void * key1,
      const void * key2)
{
   return (*(const int*)key1 == *(const int*)key2); 
}

int hashfunc (void * user, int size, const void * key)
{
   return *(const int *)key % size; 
}


static iofwdbool_t walkfunc (void * user, void * funcuser, 
      const void * key, void * data)
{
   int * table = (int*) funcuser; 
   int pos = (int*) key - (int*) user; 
   assert (pos < LOOPCOUNT && pos >= 0); 
   ++table[pos]; 
   return TRUE; 
}

int main (int argc, char ** args)
{
   int i; 
   iofwdh_t table;
   int * numbers; 


   printf ("Checking iofwdhash..."); 
   fflush (stdout); 

   numbers = (int *) malloc (LOOPCOUNT * sizeof(int));
   
   table = iofwdh_init (131, comparefunc, hashfunc, numbers);
   
   /* fill table */
   for (i=0; i<LOOPCOUNT; ++i)
   {
      numbers[i] = rand(); 
      iofwdh_add (table, &numbers[i], &numbers[i]);
   }


   /* check if all numbers can be found */
   for (i=0; i<LOOPCOUNT; ++i)
   {
      int * data; 
      test_check (iofwdh_lookup (table, &numbers[i], (void**) &data), 
            "find back previously stored item");
      test_check (data == &numbers[i], "if associated data value is correct");
   }

   /* check walkfunc */
   int * checktable = calloc (sizeof(int), LOOPCOUNT); 
   test_check (iofwdh_walktable (table, walkfunc, checktable), "if walkfunc works");

   for (i=0; i<LOOPCOUNT; ++i)
   {
      test_check (checktable[i] == 1, "if all elements were visited"); 
   }
   free (checktable); 
   

   /* check count */
   test_check (iofwdh_itemcount (table)==LOOPCOUNT, "if count is correct"); 

   /* remove some numbers */
   for (i=0; i<(LOOPCOUNT2); ++i)
   {
      test_check (iofwdh_remove (table, &numbers[i], 0), "Remove failed!");
   }
   
   /* check count */
   test_check (iofwdh_itemcount (table)==(LOOPCOUNT-LOOPCOUNT2), "if count is correct"); 


   /* See if lookup still works */
   for (i=0; i<(LOOPCOUNT2); ++i)
   {
      int exists = 0;
      int j; 
      for (j=LOOPCOUNT2+1; j<LOOPCOUNT; ++j)
      {
         if (numbers[j]==numbers[i])
         {
            exists = 1;
            break; 
         }
      }

      test_check (iofwdh_lookup (table, &numbers[i], 0) == exists,
            "if remove worked"); 
   }

   iofwdh_destroy (table); 

   printf ("All ok :)\n"); 
   return EXIT_SUCCESS; 
}

