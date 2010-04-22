#include <typeinfo>
#include "Configurable.hh"

namespace iofwdutil
{

   void Configurable::configure_if_needed (Configurable * cfg, const
         ConfigFile & config)
   {
      // See if the object implements the interface;
      try
      {
         dynamic_cast<Configurable *>(cfg)->configure (config);
      }
      catch (std::bad_cast & bd)
      {
      }
   }

}
