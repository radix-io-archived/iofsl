#include "FactoryHelper.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwd_config.h"

GENERIC_FACTORY_CLIENT_REGISTER(defasync);
GENERIC_FACTORY_CLIENT_REGISTER(zoidfs);
GENERIC_FACTORY_CLIENT_REGISTER(log);
GENERIC_FACTORY_CLIENT_REGISTER(syncadapter);
GENERIC_FACTORY_CLIENT_REGISTER(ratelimit);

GENERIC_FACTORY_CLIENT_REGISTER(requestscheduler);
GENERIC_FACTORY_CLIENT_REGISTER(hierarchical);
GENERIC_FACTORY_CLIENT_REGISTER(intervaltree);

#ifdef HAVE_ZLIB
GENERIC_FACTORY_CLIENT_REGISTER(zlib);
#endif

namespace iofwd
{
   void registerFactoryClients ()
   {
      GENERIC_FACTORY_CLIENT_CALL(defasync);
      GENERIC_FACTORY_CLIENT_CALL(zoidfs);
      GENERIC_FACTORY_CLIENT_CALL(log);
      GENERIC_FACTORY_CLIENT_CALL(syncadapter);
      GENERIC_FACTORY_CLIENT_CALL(ratelimit);

      GENERIC_FACTORY_CLIENT_CALL(requestscheduler);
      GENERIC_FACTORY_CLIENT_CALL(hierarchical);
      GENERIC_FACTORY_CLIENT_CALL(intervaltree);

#ifdef HAVE_ZLIB
      GENERIC_FACTORY_CLIENT_CALL(zlib);
#endif
   }
}

