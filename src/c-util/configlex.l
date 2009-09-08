%{

#include <string.h>
#include "configglue.h"
#include "configparser.h"
#include "tools.h"

#define YY_EXTRA_TYPE ParserParams * 

/* Disable warnings in the flex generated code */
#if defined __GNUC__
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#elif defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#endif

#define CFGP_CHECKOVERFLOW 


%}


%option reentrant stack noyywrap bison-locations yylineno
%x COMMENT STRING_LITERAL COMMENT2
%option prefix="cfgp_"
%option bison-bridge

DIGIT   [0-9]
ID      [a-zA-Z][a-zA-Z0-9,_,-]*

SECTION_OPEN  "{"
SECTION_CLOSE "}"
SEMICOLUMN    ";"

%%

"/*"                 { 
                        /* ignore comments */
                        yy_push_state(COMMENT2, yyscanner);
                     }

<COMMENT2>"*/"       {
                        /* ignore comments */
                        yy_pop_state (yyscanner);  
                     }

<COMMENT2>.+          { 
                        /* ignore comments */
                      }

"//"                 { yy_push_state(COMMENT, yyscanner); }

<COMMENT>\n          { yy_pop_state( yyscanner ); }
<COMMENT>[^\n]+      { } 

\"                   { 
                        yylval->curstringpos = 0;
                        yy_push_state(STRING_LITERAL, yyscanner);
                     }


<STRING_LITERAL>\"        { 
                                 /* saw closing quote - all done */
                                 yy_pop_state(yyscanner);
                                 CFGP_CHECKOVERFLOW;
                                 yylval->string_buf[yylval->curstringpos] = '\0';
                                 /* return string constant token type and
                                  * value to parser
                                  */
                                 return LITERAL_STRING;
                           }

<STRING_LITERAL>\n        {
               return cfgp_lex_error (yylineno,
                  "Unterminated string constant!\n");
        }

<STRING_LITERAL>\\[0-7]{1,3} {
        /* octal escape sequence */
        int result;

        (void) sscanf( yytext + 1, "%o", &result );

        if ( result > 0xff )
        {
                /* error, constant is out-of-bounds */
                return cfgp_lex_error (yylineno,
                   "Out of bounds \\0xx escape in string!\n");
        }

        yylval->string_buf[yylval->curstringpos++] = result;
        CFGP_CHECKOVERFLOW;
        }

<STRING_LITERAL>\\[0-9]+ {
        /* generate error - bad escape sequence; something
         * like '\48' or '\0777777'
         */
                return cfgp_lex_error (
                    yylineno, "Bad \\xxxx escape in string!\n");
        }

<STRING_LITERAL>\\n  { yylval->string_buf[yylval->curstringpos++] = '\n'; CFGP_CHECKOVERFLOW; }
<STRING_LITERAL>\\t  { yylval->string_buf[yylval->curstringpos++] = '\t'; CFGP_CHECKOVERFLOW; }
<STRING_LITERAL>\\r  { yylval->string_buf[yylval->curstringpos++] = '\r'; CFGP_CHECKOVERFLOW; }
<STRING_LITERAL>\\b  { yylval->string_buf[yylval->curstringpos++] = '\b'; CFGP_CHECKOVERFLOW; }
<STRING_LITERAL>\\f  { yylval->string_buf[yylval->curstringpos++] = '\f'; CFGP_CHECKOVERFLOW; }

<STRING_LITERAL>\\(.|\n) {
                yylval->string_buf[yylval->curstringpos++] = yytext[1];
                CFGP_CHECKOVERFLOW;
                }

<STRING_LITERAL>[^\\\n\"]+        {
        char *yptr = yytext;

        while ( *yptr )
        {
                yylval->string_buf[yylval->curstringpos++] = *yptr++;
                CFGP_CHECKOVERFLOW;
        }
        }

{ID}    {
                if (strlen (yytext) >= sizeof(yylval->string_buf))
                {
                        ALWAYS_ASSERT(0 && "ID too long!");
                }
                strncpy (yylval->string_buf, yytext,  sizeof (yylval->string_buf)); 
                return IDENTIFIER;
                
        }

{SECTION_OPEN}      { return OPENSECTION; }
{SECTION_CLOSE}     { return CLOSESECTION; }
{SEMICOLUMN}        { return SEMICOLUMN; }
"="                { return EQUAL_TOKEN; }
","                { return KOMMA; }
")"                { return LCLOSE; }
"("                { return LOPEN; }

[ \t]+              /* ignore whitespace */;
\n                  /* ignore EOL */;

.                   { 
                        return cfgp_lex_error (yylineno,
                             "Unexpected character: '%c'!\n");
                    }
%%

void cfgp_setfile (FILE * f, yyscan_t scanner)
{
   yyset_in (f, scanner);
}
