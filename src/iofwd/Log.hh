#ifndef IOFWD_LOG_HH
#define IOFWD_LOG_HH

#include "iofwd/service/Service.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

namespace iofwd
{
   //========================================================================

   class Config;

   /**
    * Initializes and provides access to the server log
    * Depends on Config service to obtain the log settings.
    */
   class Log : public service::Service
   {
      public:
         Log (service::ServiceManager & man);

         iofwdutil::IOFWDLogSource & getSource (const char * name);

         iofwdutil::IOFWDLogSource & getSource ();

      protected:
         boost::shared_ptr<Config> config_;
   };

   //========================================================================
}

#endif
