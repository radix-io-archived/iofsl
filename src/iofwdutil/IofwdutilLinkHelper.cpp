#include "IofwdutilLinkHelper.hh"
#include "iofwd_config.h"
#include "LinkHelper.hh"

void registerIofwdutilFactoryClients ()
{
#ifdef HAVE_ZLIB
      GENERIC_FACTORY_LINKHELPER(\
      (zlibencode) \
      (zlibdecode) \
      )
#endif
#ifdef HAVE_BZLIB
      GENERIC_FACTORY_LINKHELPER(\
      (bzlibencode) \
      (bzlibdecode) \
      )
#endif
      GENERIC_FACTORY_LINKHELPER(\
      (lzfdecode) \
      (lzfencode) \
      (copytransformencode) \
      (copytransformdecode) \
      )
}
