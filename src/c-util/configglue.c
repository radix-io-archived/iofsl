#include <stdio.h>
#include <string.h>
#include "configglue.h"

int cfgp_lex_error (int lineno, const char * msg)
{
   fprintf (stderr, "lexer error (line %u): %s", lineno, msg);
   return -1;
}

int cfgp_parser_error(ParserParams * p, const char * err, const char * loc)
{
   char buf[512];
   snprintf (buf, sizeof(buf), "Parser error (%s): %s", loc, err);
   p->error_code =1;
   p->error_string = strdup (buf);
   return -1;
}

void cfgp_initparams (ParserParams * p, ConfigHandle h)
{
   p->configfile = h;
   p->stacktop = 0;
   p->sectionstack[0] = 0;
   p->error_code = 0;
   p->error_string = 0;
}
