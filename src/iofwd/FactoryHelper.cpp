#include "FactoryHelper.hh"
#include "iofwdutil/LinkHelper.hh"
#include "zoidfs/util/ZoidFSAsync.hh"
#include "zoidfs/util/ZoidFSDefAsync.hh"

GENERIC_FACTORY_CLIENT_REGISTER(defasync);

int test;

namespace iofwd
{
   void registerFactoryClients ()
   {
      GENERIC_FACTORY_CLIENT_CALL(defasync);
   }
}

