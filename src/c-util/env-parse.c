#include <stdlib.h>
#include <string.h>
#include "env-parse.h"

int env_parse_have_debug (const char * opt)
{
   int ret =0;
   const char * str;
   const char * delim = ",";
   const char * token;
   char * nstr = 0;
   char * saveptr;

   str = getenv ("ZOIDFS_POSIX_DEBUG");
   if (!str)
   {
      ret = 0;
      goto cleanup;
   }

   nstr = strdup (str);

   token = strtok_r(nstr, delim, &saveptr);

   if (token == 0)
   {
      ret = 0;
      goto cleanup;
   }
   do
   {
      if (!strcasecmp (token, opt))
      {
         ret = 1;
         goto cleanup;
      }
   } while ((token = strtok_r(NULL, delim, &saveptr)));

cleanup:
   free (nstr);
   return ret;
}
