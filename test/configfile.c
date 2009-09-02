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

void dump_configfile (const char * filename)
{
   ConfigHandle h = txtfile_openConfig (filename); 
   if (!h)
   {
      fprintf (stderr, "Error opening file %s!\n", filename);
      return;
   }

   cf_dump (h);
   cf_free (h);
}

int main (int argc, char ** args)
{

   int flags, opt;
   int nsecs, tfnd;
   int i;

   nsecs = 0;
   tfnd = 0;
   flags = 0;
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
      dump_configfile (args[i]);
   }

   /* Other code omitted */

   exit(EXIT_SUCCESS);
}

