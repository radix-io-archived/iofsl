#include <stdio.h>
#include "txt_configfile.h"
#include "tools.h"
#include "configglue.h"
#include "configparser.h"
#include "configlex.h"
#include "configstoreadapter.h"

#define MAX_CONFIG_SIZE 10*1024*1024

/* BISON doesn't declare the prototype? */
int cfgp_parse (yyscan_t * scanner, ParserParams * param);



ConfigHandle txtfile_openConfig (const char * filename)
{
   FILE  * f ;
   long size;
   ParserParams p; 
   yyscan_t scanner;
   int reject;


   f = fopen (filename, "r");
   if (!f)
      return 0;

   /* get file size */
   fseek (f, 0, SEEK_END);
 
   size = ftell (f);

   fseek (f, 0, SEEK_SET);
   if (size > MAX_CONFIG_SIZE)
   {
      fprintf (stderr, "Config file %s too large! Not parsing!\n", filename);
      ALWAYS_ASSERT(0 && "Config file too large!");
   }

   cfgp_lex_init_extra (&p, &scanner);
   cfgp_set_in (f, scanner); 
   cfgp_initparams (&p, cfsa_create (mcs_initroot ()));
   reject = cfgp_parse(scanner, &p);
   cfgp_lex_destroy (scanner);

   fclose (f);

   return p.configfile;
}

