#ifndef IOFWD_BMIRPCHELPER_HH
#define IOFWD_BMIRPCHELPER_HH

#include "service/Service.hh"

#include <boost/scoped_ptr.hpp>

namespace rpc
{
   namespace bmi
   {
      class BMIConnector;
   }
}

namespace iofwd
{
   //========================================================================

   class BMI;
   class BMIRPCClient;
   class BMIRPCServer;

   /**
    * In the BMI RPC infrastructure, each client is also a server
    */
   class BMIRPCHelper : public service::Service
   {
      public:
         BMIRPCHelper (service::ServiceManager & m);

         virtual ~BMIRPCHelper ();

      protected:
         friend class BMIRPCClient;
         friend class BMIRPCServer;

         rpc::bmi::BMIConnector & getConnector ()
         { return *connector_; }

      protected:
         boost::shared_ptr<BMI> bmi_service_;

         boost::scoped_ptr<rpc::bmi::BMIConnector> connector_;
         

   };

   //========================================================================
}

#endif
