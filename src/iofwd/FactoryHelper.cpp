#include "FactoryHelper.hh"
#include "iofwdutil/LinkHelper.hh"
#include "zoidfs/util/ZoidFSAsync.hh"
#include "zoidfs/util/ZoidFSDefAsync.hh"

GENERIC_FACTORY_CLIENT_REGISTER(defasync);
GENERIC_FACTORY_CLIENT_REGISTER(zoidfs);
GENERIC_FACTORY_CLIENT_REGISTER(log);
GENERIC_FACTORY_CLIENT_REGISTER(syncadapter);
GENERIC_FACTORY_CLIENT_REGISTER(requestscheduler);

namespace iofwd
{
   void registerFactoryClients ()
   {
      GENERIC_FACTORY_CLIENT_CALL(defasync);
      GENERIC_FACTORY_CLIENT_CALL(zoidfs);
      GENERIC_FACTORY_CLIENT_CALL(log);
      GENERIC_FACTORY_CLIENT_CALL(syncadapter);
      GENERIC_FACTORY_CLIENT_CALL(requestscheduler);
   }
}

