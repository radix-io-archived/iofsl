#include "Log.hh"
#include "Config.hh"

#include "iofwd/service/ServiceManager.hh"
#include "iofwdutil/IOFWDLog.hh"

#include <boost/format.hpp>

SERVICE_REGISTER(iofwd::Log, log);

namespace iofwd
{
   //========================================================================

   Log::Log (service::ServiceManager & man)
      : service::Service (man),
        config_ (lookupService<Config>("config"))
   {
      // @TODO: get log configuration from config file and set IOFWDLog
      // settings accordingly
   }


   iofwdutil::IOFWDLogSource & Log::getSource ()
   {
      return iofwdutil::IOFWDLog::getSource ();
   }

   iofwdutil::IOFWDLogSource & Log::getSource (const char * name)
   {
      return iofwdutil::IOFWDLog::getSource (name);
   }

   //========================================================================
}
