#ifndef IOFWD_CHECKS_H
#define IOFWD_CHECKS_H

#include <stdio.h>
#include <stdlib.h>

inline void test_check (int cond, const char * str)
{
   if (!cond)
   {
      fprintf (stderr, "Test failed: %s!", str); 
      exit (1); 
   }
}


#endif
