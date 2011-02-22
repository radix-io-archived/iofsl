#ifndef IOFWD_BMIRPC_HH
#define IOFWD_BMIRPC_HH

#include "iofwd/service/Service.hh"
#include "RPCClient.hh"

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
   class Log;
   class Config;
   class BMIRPCHelper;

   /**
    * BMI Client service
    */
   class BMIRPCClient : public service::Service,
                  public RPCClient
   {
      public:
         BMIRPCClient (service::ServiceManager & m);

         virtual ~BMIRPCClient ();

         unsigned int rank () const;

         unsigned int size () const;

         rpc::CommChannel operator () (const rpc::RPCKey & key,
                                       unsigned int rank);

      protected:
         boost::shared_ptr<Log> log_service_;
         boost::shared_ptr<Config> config_service_;
         boost::shared_ptr<BMI> bmi_service_;
         boost::shared_ptr<BMIRPCHelper> rpchelper_service_;

         rpc::bmi::BMIConnector & connector_;
   };

   //========================================================================
}

#endif
