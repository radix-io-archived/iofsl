#include <stdio.h>
#include "configglue.h"

int cfgp_lex_error (int lineno, const char * msg)
{
   fprintf (stderr, "lexer error (line %u): %s", lineno, msg);
   return -1;
}

int cfgp_parser_error(const char * err)
{
   fprintf (stderr, "parser error: %s", err);
   return -1;
}

void cfgp_initparams (ParserParams * p, ConfigHandle h)
{
   p->configfile = h;
   p->stacktop = 0;
   p->sectionstack[0] = 0;
}
