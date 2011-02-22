#include "BMIRPCServer.hh"
#include "BMIRPCHelper.hh"

#include "iofwdutil/IOFWDLog.hh"

#include "rpc/RPCRegistry.hh"
#include "rpc/RPCServer.hh"
// #include "rpc/bmi/BMIConnector.hh"

SERVICE_REGISTER(iofwd::BMIRPCServer, bmirpcserver);

using boost::format;

namespace iofwd
{
   //========================================================================

   BMIRPCServer::BMIRPCServer (service::ServiceManager & m)
      : service::Service (m),
        log_service_ (lookupService<Log>("log")),
        rpchelper_service_ (lookupService<BMIRPCHelper>("bmirpchelper")),
        registry_ (new rpc::RPCRegistry ()),
        rpcserver_ (new rpc::RPCServer (*registry_)),
        log_ (log_service_->getSource ("rpc"))
   {
      // rpchelper_service_->getConnector().registerExec (rpcserver_.get ());
   }

   BMIRPCServer::~BMIRPCServer ()
   {
      // rpchelper_service_->getConnector().registerExec (0);
   }

   void BMIRPCServer::registerRPC (const rpc::RPCKey & key, const
         rpc::RPCHandler & handler)
   {
      ZLOG_DEBUG (log_, format("Register RPC key %lu") % key);
   }

   void BMIRPCServer::unregisterRPC (const rpc::RPCKey & key)
   {
      ZLOG_DEBUG (log_, format("Unregister RPC key %lu") % key);
   }

   //========================================================================
}
