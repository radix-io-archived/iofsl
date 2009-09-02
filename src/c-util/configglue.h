#ifndef C_UTIL_CONFIGGLUE_H
#define C_UTIL_CONFIGGLUE_H

/* Common header for parser and lexer */

#include "configfile.h"


typedef struct
{
   ConfigHandle configfile;

   SectionHandle sectionstack [20];
   unsigned int stacktop;

   /* used to construct a multival key */
   struct 
   { 
            char ** keyvals;
            unsigned int count;
            unsigned int maxsize;
   }; 

} ParserParams;

void cfgp_initparams (ParserParams * p, ConfigHandle h);

int cfgp_parser_error ( const char* str); 

int cfgp_lex_error (int lineno,const char * msg); 

#endif
