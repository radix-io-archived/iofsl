/**
 * Test program that tries to read a configfile and dumps the tree to stdout
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "c-util/txt_configfile.h"
#include "c-util/configfile.h"

static unsigned int opt_verbose;

void dump_configfile (const char * filename, char ** err)
{
   char * err2 =0;

   ConfigHandle h = txtfile_openConfig (filename, err);
   if (!h)
      return;

   if (*err)
   {
      fprintf (stderr, "NOTE: Error parsing file: %s. Output up to error follows.\n", 
            *err);
   }

   /* no need to check for error, *err will indicate error
    * and we always need to free h */
   cf_dump (h, ROOT_SECTION, &err2);
   if (err2)
   {
      fprintf (stderr, "Error dumping configtree: %s\n", err2);
   }

   if (*err && err2)
   {
      free (err2); err2=0;
   }
   if (!*err && err2)
      *err = err2;

   cf_free (h);
}

int main (int argc, char ** args)
{
   int opt;
   int nsecs, tfnd;
   int i;

   nsecs = 0;
   tfnd = 0;
   while ((opt = getopt(argc, args, "v")) != -1) {
      switch (opt) {
         case 'v':
            opt_verbose = 1;
            break;
         default: /* '?' */
            fprintf(stderr, "Usage: %s [-v] configfile\n",
                  args[0]);
            exit(EXIT_FAILURE);
      }
   }

   if (optind >= argc) {
      fprintf(stderr, "Expected argument after options\n");
      exit(EXIT_FAILURE);
   }

   for (i=optind; i<argc; ++i)
   {
      char * err =0;
      dump_configfile (args[i], &err);
      if (err)
      {
         fprintf (stderr, "Error dumping config file '%s': %s\n", 
               args[i], err);
         free (err);
      }
   }

   exit(EXIT_SUCCESS);
}

