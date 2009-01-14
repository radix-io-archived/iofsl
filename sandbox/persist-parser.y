%{
#include <stdio.h>
#include "persist-parser-shared.h"

void yyerror(const char *s);
int yylex(void);
%}

%union{
  const char *	filename;
  unsigned long number; 
  const char * identifier; 
}

%start	input 

%token  <number>        TOKEN_NUMBER
%token	<filename>	TOKEN_FILENAME
%token                  TOKEN_NEWLINE
%token  <identifier>    TOKEN_IDENTIFIER


%%

input: /* empty */ 
     | commandline 
;

startcommand: TOKEN_IDENTIFIER
            { if (!start_command ($1)) YYABORT; }
;

param: TOKEN_FILENAME { if (!param_command ($1)) YYABORT; }
;

params: /* empty */
      | params param 
      ; 


commandline: 
           startcommand params 
            { if (!end_command ()) YYABORT;  }  
;

%%

    void yyerror (char const *s)
     {
       fprintf (stderr, "%s\n", s);
     }

