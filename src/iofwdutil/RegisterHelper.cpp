#include "RegisterHelper.hh"
#include "LinkHelper.hh"
#include "iofwd_config.h"

#ifdef HAVE_ZLIB
GENERIC_FACTORY_CLIENT_REGISTER(zlib);
GENERIC_FACTORY_CLIENT_REGISTER(zlibencode);
GENERIC_FACTORY_CLIENT_REGISTER(zlibdecode);
#endif

#ifdef HAVE_BZLIB
GENERIC_FACTORY_CLIENT_REGISTER(bzlibencode);
GENERIC_FACTORY_CLIENT_REGISTER(bzlibdecode);
#endif

GENERIC_FACTORY_CLIENT_REGISTER(lzfencode);
GENERIC_FACTORY_CLIENT_REGISTER(lzfdecode);

GENERIC_FACTORY_CLIENT_REGISTER(copytransformencode);
GENERIC_FACTORY_CLIENT_REGISTER(copytransformdecode);

namespace iofwdutil
{
   void registerIofwdutilFactoryClients ()
   {
#ifdef HAVE_ZLIB
      GENERIC_FACTORY_CLIENT_CALL(zlib);
      GENERIC_FACTORY_CLIENT_CALL(zlibencode);
      GENERIC_FACTORY_CLIENT_CALL(zlibdecode);
#endif

      GENERIC_FACTORY_CLIENT_CALL(lzfencode);
      GENERIC_FACTORY_CLIENT_CALL(lzfdecode);

      GENERIC_FACTORY_CLIENT_CALL(copytransformencode);
      GENERIC_FACTORY_CLIENT_CALL(copytransformdecode);

#ifdef HAVE_BZLIB
      GENERIC_FACTORY_CLIENT_CALL(bzlibencode);
      GENERIC_FACTORY_CLIENT_CALL(bzlibdecode);
#endif
   }
}

