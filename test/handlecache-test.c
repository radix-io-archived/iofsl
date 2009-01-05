#include <stdlib.h>
#include <stdio.h>
#include "handlecache.h"
#include "checks.h"

static unsigned long handle_num = 0; 
static unsigned int handle_open = 0; 

#define MAX_OPEN 91

int hc_test_open (const char * path, hc_value_t * dst, int flags,
      iofwdbool_t upgrade)
{
   ++handle_num;
   ++handle_open; 
   test_check (handle_open >=0 && handle_open <= MAX_OPEN,
         "Invalid number of open handles!"); 
   *dst = (hc_value_t) handle_num; 
   printf ("Opening '%s' (handle %p, flags %i, upgrade %i)\n", path, *dst,
         flags, upgrade);
   return 1; 
}

int hc_test_close (hc_value_t * dst)
{
   --handle_open; 
   test_check (handle_open >=0 && handle_open <= MAX_OPEN,
         "Invalid number of open handles!"); 
   printf ("Closing handle %p\n", *dst); 
}

int main (int argc, char ** args)
{
   handlecache_init (MAX_OPEN, hc_test_open, hc_test_close); 
   char buf[256]; 
   int i; 

   for (i=0; i<300; ++i)
   {
      sprintf (buf, "file%i", i); 
      printf ("Opening file %s\n", buf); 
      //handlecache_get (buf, HC_READ); 
   }

   //handlecache_destroy (); 
   return EXIT_SUCCESS; 
}
