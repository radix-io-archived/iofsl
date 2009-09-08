#include <assert.h>
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

static void show_indent (FILE * f, unsigned int ind)
{
   unsigned int i;

   for (i=0; i<ind; ++i)
      fprintf (f, " "); 
}


static void dump_section (FILE * f, ConfigHandle h, SectionHandle s, unsigned int indent)
{
   unsigned int sectionsize;
   size_t count;
   unsigned int i;

   cf_getSectionSize (h, s, &sectionsize);

   count = sectionsize;
  
   SectionEntry entries[sectionsize];

   cf_listSection (h, s, &entries[0], &count);
   assert (sectionsize == count);

   for (i=0; i<count; ++i)
   {
      show_indent (f,indent);
      switch (entries[i].type)
      {
         case SE_SECTION:
            {
               SectionHandle newsec;

               fprintf (f, "%s { \n", entries[i].name);

               cf_openSection (h, s, entries[i].name, &newsec);
               dump_section (f, h, newsec, indent + 2);
               show_indent (f, indent);
               fprintf (f, "}\n");
               cf_closeSection (h, newsec);
               break;
            }
         case SE_KEY:
            {
               char buf[255];
               cf_getKey (h, s, entries[i].name, &buf[0], sizeof(buf));
               fprintf (f, "%s = \"%s\";\n", entries[i].name, buf); 
               break;
            }
         case SE_MULTIKEY:
            {
               char ** ptrs;
               size_t size;
               size_t j;
               cf_getMultiKey (h, s, entries[i].name, &ptrs, &size);
               fprintf (f,"%s = (", entries[i].name);
               for (j=0; j<size; ++j)
               {
                  fprintf (f,"\"%s\" ", (ptrs[j] ? ptrs[j] : ""));
                  free (ptrs[j]);
                  if ((j+1) < size)
                     fprintf (f,", ");
               }
               fprintf (f, ");\n");
               free (ptrs);
            }
      }
      free ((void*)entries[i].name);
   }
}


void txtfile_writeConfig (ConfigHandle cf, FILE * f)
{
   dump_section (f, cf, ROOT_SECTION, 0);
}

ConfigHandle txtfile_openStream (FILE * f, char ** err)
{
   long size;
   ParserParams p; 
   yyscan_t scanner;
   int reject;
   char buf[512];
   
   /* get file size */
   fseek (f, 0, SEEK_END);
 
   size = ftell (f);

   fseek (f, 0, SEEK_SET);
   if (size > MAX_CONFIG_SIZE)
   {
      fprintf (stderr, "Config file too large! Not parsing!\n");
      ALWAYS_ASSERT(0 && "Config file too large!");
   }

   cfgp_lex_init_extra (&p, &scanner);
   cfgp_set_in (f, scanner); 
   cfgp_initparams (&p, cfsa_create (mcs_initroot ()));
   reject = cfgp_parse(scanner, &p);
   cfgp_lex_destroy (scanner);

   /* either we have a valid confighandle or we have a parser error... */
   /* not true: we can have a partial config tree */
   // ALWAYS_ASSERT((p.error_code || p.configfile) && (!p.error_code || !p.configfile));

   /* If ther parser failed we need to have an error code */
   ALWAYS_ASSERT(!reject || p.parser_error_code || p.lexer_error_code);

   if (!cfgp_parse_ok (&p, buf, sizeof(buf)))
   {
      *err = strdup (buf);
   }
   else
   {
      ALWAYS_ASSERT(!p.parser_error_string);
      ALWAYS_ASSERT(!p.lexer_error_string);
      if (err) *err = 0;
   }

   cfgp_freeparams (&p);

   return p.configfile;
 }

ConfigHandle txtfile_openConfig (const char * filename, char ** error)
{
   FILE  * f;
   ConfigHandle ret;


   f = fopen (filename, "r");
   if (!f)
   {
      char buf[255];
      strerror_r (errno, buf, sizeof(buf));
      *error = strdup (buf);
      return 0;
   }

   ret = txtfile_openStream (f, error);

   fclose (f);

   return ret;
}

