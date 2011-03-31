
#include "iofwdutil/LinkHelper.hh"
#include "iofwdclient/LinkHelper.hh"
#include "iofwd/service/Service.hh"

#include "iofwd_config.h"

void registerIofwdClients ()
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
            (bmirpcclient) \
            (log) \
            (rpcclient) \
            (net) \
      );

//#ifdef HAVE_FTB
//      SERVICE_LINKHELPER( \
//            (ftb) \
//         );
//#endif

}
