#include "Service.hh"
#include "ServiceManager.hh"

namespace iofwd
{
   namespace service
   {
      //=====================================================================

      Service::Service (ServiceManager & m)
         : manager_ (m)
      {
      }

      boost::shared_ptr<Service> Service::lookupServiceHelper (const
            std::string & name)
      {
         return manager_.lookupService (name);
      }

      Service::~Service ()
      {
      }

      //=====================================================================
   }
}
