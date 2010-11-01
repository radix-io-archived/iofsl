#include "iofwd/RegisterHelper.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwd_config.h"

GENERIC_FACTORY_CLIENT_REGISTER(requestscheduler);
GENERIC_FACTORY_CLIENT_REGISTER(hierarchical);
GENERIC_FACTORY_CLIENT_REGISTER(intervaltree);

namespace iofwd
{
   void registerIofwdFactoryClients ()
   {
      GENERIC_FACTORY_CLIENT_CALL(requestscheduler);
      GENERIC_FACTORY_CLIENT_CALL(hierarchical);
      GENERIC_FACTORY_CLIENT_CALL(intervaltree);
   }
}

