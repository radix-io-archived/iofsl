#include <stdio.h>
#include <stdlib.h>
#include "iofwdhash.h"

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


inline void check (int cond, const char * str)
{
   if (!cond)
   {
      fprintf (stderr, "Test failed: %s!", str); 
      exit (1); 
   }
}

int main (int argc, char ** args)
{
   int i; 
   iofwdh_t table;
   int * numbers; 


   printf ("Checking iofwdhash..."); 
   fflush (stdout); 
   table = iofwdh_init (131, comparefunc, hashfunc, 0);

   /* fill table */
   numbers = (int *) malloc (LOOPCOUNT * sizeof(int));
   for (i=0; i<LOOPCOUNT; ++i)
   {
      numbers[i] = rand(); 
      iofwdh_add (table, &numbers[i], &numbers[i]);
   }

   /* check if all numbers can be found */
   for (i=0; i<LOOPCOUNT; ++i)
   {
      int * data; 
      check (iofwdh_lookup (table, &numbers[i], (void**) &data), 
            "find back previously stored item");
      check (data == &numbers[i], "if associated data value is correct");
   }

   /* check count */
   check (iofwdh_itemcount (table)==LOOPCOUNT, "if count is correct"); 

   /* remove some numbers */
   for (i=0; i<(LOOPCOUNT2); ++i)
   {
      check (iofwdh_remove (table, &numbers[i], 0), "Remove failed!");
   }
   
   /* check count */
   check (iofwdh_itemcount (table)==(LOOPCOUNT-LOOPCOUNT2), "if count is correct"); 


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

      check (iofwdh_lookup (table, &numbers[i], 0) == exists,
            "if remove worked"); 
   }

   iofwdh_destroy (table); 

   printf ("All ok :)\n"); 
   return EXIT_SUCCESS; 
}

