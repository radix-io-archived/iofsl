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

   int                  error_code;

   /* if error_code != 0 the user needs to free error_string */
   char *         error_string;
} ParserParams;

void cfgp_initparams (ParserParams * p, ConfigHandle h);

int cfgp_parser_error (ParserParams * p, const char* str, const char *
      lcoation); 

int cfgp_lex_error (int lineno,const char * msg); 

#endif
