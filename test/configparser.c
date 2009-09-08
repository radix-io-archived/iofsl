#include <stdio.h>
#include "c-util/configglue.h"
#include "c-util/configparser.h"
#include "c-util/configlex.h"
#include "c-util/configstoreadapter.h"
#include "c-util/configglue.h"

/* BISON doesn't declare the prototype? */
int cfgp_parse (yyscan_t * scanner, ParserParams * param);


int main()
{
  yyscan_t scanner;
  ParserParams p;

  int reject;

  while(!feof(stdin))
  {
    printf("enter a string to be parsed: \n");

    cfgp_lex_init_extra (&p, &scanner);

    cfgp_initparams (&p, cfsa_create (mcs_initroot ()));

    reject = cfgp_parse(scanner, &p);
    if(reject)
    {
      printf("rejected: %s \n", p.error_string);
      free (p.error_string);
    }
    else
    {
      printf("accepted \n");
    }

    cf_free (p.configfile);
  }
  return 0;
} 

