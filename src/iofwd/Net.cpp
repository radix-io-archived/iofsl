#include "iofwd/Net.hh"
#include "iofwd/service/ServiceManager.hh"
#include "iofwd/Log.hh"
#include "iofwd/BMI.hh"
#include "iofwd/Config.hh"
#include "iofwd/ConfigException.hh"

#include "net/Communicator.hh"
#include "net/bmi/BMIConnector.hh"
#include "net/loopback/LoopbackConnector.hh"

SERVICE_REGISTER(iofwd::Net, net);

namespace iofwd
{
   //========================================================================

   Net::Net (service::ServiceManager & m)
      : service::Service (m),
        config_service_ (lookupService<Config>("config")),
        log_service_ (lookupService<Log>("log"))
   {
      // Lookup net type
      iofwdutil::ConfigFile config =
         config_service_->getConfig().openSection ("net");
      std::string type = config.getKeyDefault ("type", "local");

      if (type == "bmi")
      {
         bmi_service_ = lookupService<BMI>("bmi");
         net_.reset (new net::bmi::BMIConnector (bmi_service_->get (),
                  log_service_->getSource ("net")));
      }
      else if (type == "local")
      {
         net_.reset (new net::loopback::LoopbackConnector ());

      }
      else
      {
         ZTHROW (ConfigException () << ce_key ("type") <<
               ce_environment ("net"));
      }
   }

   net::ConstCommunicatorHandle Net::getServerComm () const
   {
      return servercomm_;
   }

   Net::~Net ()
   {
   }


   //========================================================================
}
