#ifndef C_UTIL_CONFIGSTOREADAPTER_H
#define C_UTIL_CONFIGSTOREADAPTER_H

#include "configfile.h"
#include "configstore.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Create a new configfile interface backed by a configstore */
ConfigHandle cfsa_create (mcs_entry * e);


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
