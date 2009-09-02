#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include "configfile.h"
#include "txt_configfile.h"

void cf_dump (ConfigHandle cf)
{
   txtfile_writeConfig (cf, stdout);
}
