#include <stdio.h>
#include "tools.h"

void always_assert_error (const char * expr, const char * file, int lineno)
{
   fprintf (stderr, "Assertion '%s' failed (%s:%i)!\n", expr, file, lineno);
}
