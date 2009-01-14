%{
#include <string.h>
#include "persist-parser.h"
#include "persist-parser-shared.h"

%}

number          [0-9][0-9]*
filenamechar    [^ \t]
identifier      [a-z,A-Z][a-z,A-Z]*
slash           "/"
filename        {slash}{filenamechar}*

%option noyywrap nounput batch 


%%

{number}        { yylval.number = atoi(yytext); return TOKEN_NUMBER; } 

{filename}	{ yylval.filename = strdup(yytext); return TOKEN_FILENAME; }

{identifier}    { yylval.identifier = strdup(yytext); return TOKEN_IDENTIFIER; }

[ \t]*		{}

\n              { return TOKEN_NEWLINE; }

.               {fprintf (stderr,"Unhandled token: %s\n", yytext); }


