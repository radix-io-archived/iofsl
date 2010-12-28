#ifndef IOFWD_BMI_HH
#define IOFWD_BMI_HH

#include "iofwd/service/Service.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

#include <boost/scoped_ptr.hpp>
#include <vector>
#include <string>

extern "C"
{
#include <bmi.h>
}

namespace iofwdevent
{
   class BMIResource;
}

namespace iofwdutil
{
   class ConfigFile;
}

namespace iofwd
{
   //========================================================================


   // Fwd declaration of some services
   class Config;
   class Log;

   /**
    * BMI Service.
    *
    * Looks into config file and initializes BMI with the correct parameters.
    * Provides additional services, such as group services (determining rank
    * in set of forwarding servers), buffer management, ...
    *
    * @TODO: Move BMI configuration options in here as much as possible
    *        (for example, buffer/memory management)
    *
    */
   struct BMI : public service::Service
   {
      public:
         BMI (service::ServiceManager & m);

         ~BMI ();

         iofwdevent::BMIResource & get ()
         { return *resource_; }

         unsigned int getServerCount () const
         { return servernames_.size (); }

         unsigned int getServerRank () const
         { return serverrank_; }

         const std::string & getServer (unsigned int pos) const
         { return servernames_[pos]; }


      protected:

         /// Initialize BMI
         void init ();

         /// shutdown BMI
         void done ();

         std::string multiServerMode (const iofwdutil::ConfigFile & c);

         std::string singleServerMode (const iofwdutil::ConfigFile & c);

         // BMI_addr_t lookupAddr (const std::string & s) const;

      protected:
         // Services
         //   We need these to ensure the services remain alive while we use
         //   the log / config file
         boost::shared_ptr<Config> config_service_;
         boost::shared_ptr<Log> log_service_;

         boost::scoped_ptr<iofwdevent::BMIResource> resource_;
         const iofwdutil::ConfigFile & config_;
         iofwdutil::IOFWDLogSource & log_;

         unsigned int serverrank_;

         // std::vector<BMI_addr_t> servers_;
         std::vector<std::string> servernames_;
   };

   //========================================================================
}

#endif
