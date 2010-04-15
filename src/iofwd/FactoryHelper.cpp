#include "FactoryHelper.hh"
#include "iofwdutil/LinkHelper.hh"

#include "zoidfs/util/ZoidFSAsync.hh"
#include "zoidfs/util/ZoidFSDefAsync.hh"

namespace iofwd
{

   void registerFactoryClients ()
   {
      GENERIC_FACTORY_CLIENT_REGISTER(std::string,
                                      zoidfs::util::ZoidFSAsync,
                                      zoidfs::util::ZoidFSDefAsync);
   }

}
