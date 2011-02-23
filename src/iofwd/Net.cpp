#include "iofwd/Net.hh"
#include "iofwd/service/ServiceManager.hh"
#include "iofwd/Log.hh"
#include "iofwd/BMI.hh"
#include "iofwd/Config.hh"
#include "iofwd/ConfigException.hh"

#include "iofwdevent/SingleCompletion.hh"

#include "net/Group.hh"
#include "net/Communicator.hh"
#include "net/bmi/BMIConnector.hh"
#include "net/loopback/LoopbackConnector.hh"

#include <vector>

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

         std::vector<std::string> s;
         for (size_t i =0; i<bmi_service_->getServerCount (); ++i)
            s.push_back (bmi_service_->getServer (i));
         createServerComm (s, bmi_service_->getServerRank ());
      }
      else if (type == "local")
      {
         net_.reset (new net::loopback::LoopbackConnector ());

         std::vector<std::string> s;
         s.push_back ("local");
         createServerComm (s, 0);
      }
      else
      {
         ZTHROW (ConfigException () << ce_key ("type") <<
               ce_environment ("net"));
      }
   }

   void Net::createServerComm (const std::vector<std::string> & l,
         size_t myrank)
   {
      iofwdevent::SingleCompletion block;

      net::GroupHandle g = new net::Group ();

      for (size_t i=0; i<l.size(); ++i)
      {
         net::AddressPtr a;

         block.reset ();
         net_->lookup (l[i].c_str(), &a, block);
         block.wait ();

         g->push_back (a);
      }
      servercomm_.reset (new net::Communicator (g, myrank));
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
