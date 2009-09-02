#ifndef C_UTIL_CONFIGSTOREADAPTER_H
#define C_UTIL_CONFIGSTOREADAPTER_H

#include "configfile.h"
#include "configstore.h"

/* Create a new configfile interface backed by a configstore */
ConfigHandle cfsa_create (mcs_entry * e);

#endif
