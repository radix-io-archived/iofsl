/**
 * For each of the config files given on the command line,
 * try to parse it, dump to file and reread. Compare to original.
 */
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <CUnit/Basic.h>
#include "c-util/txt_configfile.h"
#include "c-util/configfile.h"

static unsigned int opt_verbose;

static unsigned int       argcount;
static const char **      fileargs;
static FILE *             tempfile;

static void clearfile (FILE * f)
{
   rewind (f);
   if (ftruncate (fileno (f), 0) < 0)
   {
      fprintf (stderr, "ftruncate failed!\n");
      exit (1);
   }
}

void dump_configfile (const char * filename)
{
   char * error;
   ConfigHandle h2 = 0;
   ConfigHandle h1 = txtfile_openConfig (filename, &error);

   if (!h1)
   {
      fprintf (stderr, "Error opening file %s: %s!\n", filename, error);
   }
   if (error)
      fprintf (stderr, "Error reading config file %s: %s\n", filename, error);

   CU_ASSERT_PTR_NULL(error);
   CU_ASSERT_PTR_NOT_NULL(h1);

   /* dump to file */
   clearfile (tempfile);
   txtfile_writeConfig (h1, ROOT_SECTION, tempfile, &error);
   if (error)
      fprintf (stderr, "Error writing configfile to temp file: %s\n", error);

   rewind (tempfile);
   h2 = txtfile_openStream (tempfile, &error);
   if (error)
      fprintf (stderr, "Error opening config file: %s!\n", error);
   CU_ASSERT_PTR_NOT_NULL(h2);

   CU_ASSERT_TRUE(cf_equal (h1, h2));

   /*cf_dump (h1);
   cf_dump (h2); */

   cf_free (h2);
   cf_free (h1);
}


static int suite_init ()
{
   tempfile = tmpfile ();
   if (!tempfile)
      fprintf (stderr, "Error opening temporary file!\n");
   return 0;
}

static int suite_done ()
{
   fclose (tempfile);
   return 0;
}

static void testconfig ()
{
   unsigned int i;
   for (i=0; i<argcount; ++i)
   {
        dump_configfile (fileargs[i]);
   }
}

int main (int argc, char ** args)
{

   int opt;
   int nsecs, tfnd;
   CU_pSuite pSuite = NULL;

   nsecs = 0;
   tfnd = 0;
   while ((opt = getopt(argc, args, "v")) != -1) {
      switch (opt) {
         case 'v':
            opt_verbose = 1;
            break;
         default: /* '?' */
            fprintf(stderr, "Usage: %s [-v] configfile... \n",
                  args[0]);
            exit(EXIT_FAILURE);
      }
   }

   if (optind >= argc) {
      fprintf(stderr, "Expected argument after options\n");
      exit(EXIT_FAILURE);
   }

   argcount = argc - optind;
   fileargs = (const char **) &args[optind];

   /* initialize the CUnit test registry */
   if (CUE_SUCCESS != CU_initialize_registry())
      return CU_get_error();

   /* add a suite to the registry */
   pSuite = CU_add_suite("Config parser/dumper", suite_init, suite_done);
   if (NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
   }

   /* add the tests to the suite */
   if (
        (NULL == CU_add_test(pSuite, "check if config files can be parsed&reconstructed", (CU_TestFunc) testconfig))
      )
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

