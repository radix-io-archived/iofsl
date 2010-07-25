#include "iofwdutil/RegisterHelper.hh"
#include "iofwdutil/LinkHelper.hh"

GENERIC_FACTORY_CLIENT_REGISTER(defasync);
GENERIC_FACTORY_CLIENT_REGISTER(zoidfs);
GENERIC_FACTORY_CLIENT_REGISTER(log);
GENERIC_FACTORY_CLIENT_REGISTER(syncadapter);
GENERIC_FACTORY_CLIENT_REGISTER(ratelimit);

namespace zoidfs
{
   namespace util
   {

      void registerZoidfsUtilFactoryClients ()
      {
         GENERIC_FACTORY_CLIENT_CALL(defasync);
         GENERIC_FACTORY_CLIENT_CALL(zoidfs);
         GENERIC_FACTORY_CLIENT_CALL(log);
         GENERIC_FACTORY_CLIENT_CALL(syncadapter);
         GENERIC_FACTORY_CLIENT_CALL(ratelimit);
      }

   }
}

