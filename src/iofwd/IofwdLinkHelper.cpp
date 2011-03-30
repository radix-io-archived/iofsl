#include "IofwdLinkHelper.hh"
#include "iofwdutil/LinkHelper.hh"
#include "iofwd/service/Service.hh"

#include "iofwd_config.h"

/*
 * List the modules that need to be registered below...
 * No particular order required
 */

void registerIofwdFactoryClients ()
{
      GENERIC_FACTORY_LINKHELPER( \
            (defasync) \
            (zoidfs) \
            (log) \
            (syncadapter) \
            (ratelimit) \
            (requestscheduler) \
            (hierarchical) \
            (intervaltree) \
      );

      // Register services
      SERVICE_LINKHELPER( \
            (config) \
            (bmi) \
            (log) \
            (iofwdserver) \
            (timer) \
            (rpcclient) \
            (rpcserver) \
            (net) \
            (bmifrontend) \
            (rpctest) \
            (iofslclientrpc) \
      );

#ifdef HAVE_FTB
      SERVICE_LINKHELPER( \
            (ftb) \
         );
#endif

}

/*
 * List the modules that need to be registered below...
 * No particular order required
 */


