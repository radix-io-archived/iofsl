#ifndef IOFWD_NET_HH
#define IOFWD_NET_HH

#include "iofwd/service/Service.hh"
#include "net/Net-fwd.hh"

#include <vector>
#include <boost/scoped_ptr.hpp>

namespace iofwd
{
   //========================================================================

   class Config;
   class Log;
   class BMI;

   class Net : public service::Service
   {
      public:
         Net (service::ServiceManager & m);

         // Forwarder to net::Net
         net::Net * operator -> ()
         { return getNet (); }

         net::Net * getNet ()
         { return net_.get(); }


         net::ConstCommunicatorHandle getServerComm () const;

         int getServerRank();

         virtual ~Net ();

      protected:
         void createServerComm (const std::vector<std::string> & l, size_t
               myrank);

      protected:
         boost::shared_ptr<Config> config_service_;
         boost::shared_ptr<Log> log_service_;
         boost::shared_ptr<BMI> bmi_service_;

         boost::scoped_ptr<net::Net> net_;

         net::ConstCommunicatorHandle servercomm_;

         std::string type_;
   };

   //========================================================================
}

#endif
