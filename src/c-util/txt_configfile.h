#ifndef C_UTIL_TXTFILE_CONFIGFILE_H
#define C_UTIL_TXTFILE_CONFIGFILE_H

#include <stdio.h>
#include "configfile.h"

/**
 * ConfigFile implementation that stores data in a text file 
 */

ConfigHandle txtfile_openConfig (const char * filename);

ConfigHandle txtfile_openStream (FILE * f);

void txtfile_writeConfig (ConfigHandle h, FILE * out);

#endif
