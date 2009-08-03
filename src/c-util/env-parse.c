#include <stdlib.h>
#include <string.h>
#include "env-parse.h"

int env_parse_have_debug (const char * opt)
{
   const char * str;
   const char * delim = ","; 
   str = getenv ("ZOIDFS_POSIX_DEBUG"); 
   if (!str)
      return 0; 
   char * saveptr; 
   const char * token = strtok_r(str, delim, &saveptr); 
   if (token == 0)
      return 0; 
   do
   {
      if (!strcmp (token, opt))
         return 1; 
   } while ((token = strtok_r(NULL, delim, &saveptr))); 
   return 0; 
}
