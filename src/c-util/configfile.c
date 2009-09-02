#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include "configfile.h"

void show_indent (FILE * f, unsigned int ind)
{
   unsigned int i;

   for (i=0; i<ind; ++i)
      fprintf (f, " "); 
}

void dump_section (FILE * f, ConfigHandle h, SectionHandle s, unsigned int indent)
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

               printf ("%s [subsection]\n", entries[i].name);

               cf_openSection (h, s, entries[i].name, &newsec);
               dump_section (f, h, newsec, indent + 2);
               cf_closeSection (h, newsec);
               break;
            }
         case SE_KEY:
            {
               char buf[255];
               cf_getKey (h, s, entries[i].name, &buf[0], sizeof(buf));
               printf ("%s = \"%s\"\n", entries[i].name, buf); 
               break;
            }
         case SE_MULTIKEY:
            {
               char ** ptrs;
               size_t size;
               size_t j;
               cf_getMultiKey (h, s, entries[i].name, &ptrs, &size);
               printf ("%s = ", entries[i].name);
               for (j=0; j<size; ++j)
               {
                  printf ("\"%s\" ", (ptrs[j] ? ptrs[j] : "(null)"));
                  free (ptrs[j]);
               }
               printf ("\n");
               free (ptrs);
            }
      }
      free ((void*)entries[i].name);
   }
}


void cf_dump (ConfigHandle cf)
{
   FILE * f = stdout;
   
   dump_section (f, cf, ROOT_SECTION, 0);
}
