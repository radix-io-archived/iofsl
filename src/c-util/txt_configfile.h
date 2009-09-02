#ifndef C_UTIL_TXTFILE_CONFIGFILE_H
#define C_UTIL_TXTFILE_CONFIGFILE_H

#include "configfile.h"

/**
 * ConfigFile implementation that stores data in a text file 
 */

ConfigHandle txtfile_openConfig (const char * filename);


#endif
