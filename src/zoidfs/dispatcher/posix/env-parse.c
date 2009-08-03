#include <stdlib.h>
#include <string.h>
#include "env-parse.h"

int env_parse_have_debug (const char * opt)
{
   const char * delim = ","; 
   /* strtok_r modifies string */
   char * str = NULL;
   int ret = 0;  
   
   if (getenv ("ZOIDFS_POSIX_DEBUG"))
   {
     str = strdup(getenv ("ZOIDFS_POSIX_DEBUG")); 
   }
   else
   {
      ret = 0; 
      goto leave; 
   }

   char * saveptr; 
   const char * token = strtok_r(str, delim, &saveptr); 

   if (token == 0)
   {
      ret = 0; 
      goto leave; 
   }

   do
   {
      if (!strcmp (token, opt))
      {
         ret = 1; 
         goto leave; 
      }
   } while ((token = strtok_r(NULL, delim, &saveptr))); 

 leave:
   free (str); 
   return ret; 
}
