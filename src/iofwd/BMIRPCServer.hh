#ifndef IOFWD_BMIRPCSERVER_HH
#define IOFWD_BMIRPCSERVER_HH

#include "iofwd/service/Service.hh"
#include "RPCServer.hh"
#include "Log.hh"
#include "BMI.hh"
#include "Config.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"

namespace rpc
{
   class RPCServer;
   class RPCRegistry;
}

namespace iofwd
{
   //========================================================================

   class BMIRPCHelper;

   class BMIRPCServer : public service::Service,
                        public RPCServer
   {
      public:
         BMIRPCServer (service::ServiceManager & );

         void registerRPC (const rpc::RPCKey & key, const rpc::RPCHandler &
               handler);

         void unregisterRPC (const rpc::RPCKey & key);

         virtual ~BMIRPCServer ();

      protected:
         boost::shared_ptr<Log> log_service_;
         //boost::shared_ptr<Config> config_service_;
         boost::shared_ptr<BMIRPCHelper> rpchelper_service_;

         boost::scoped_ptr<rpc::RPCRegistry> registry_;
         boost::scoped_ptr<rpc::RPCServer> rpcserver_;

         iofwdutil::IOFWDLogSource & log_;
   };

   //========================================================================
}


#endif

