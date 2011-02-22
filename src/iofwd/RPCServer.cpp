#include "iofwd/RPCServer.hh"

#include "iofwd/service/ServiceManager.hh"

#include "iofwd/Log.hh"
#include "iofwd/Net.hh"

#include "rpc/RPCRegistry.hh"
#include "rpc/RPCServer.hh"

#include "iofwdutil/IOFWDLog.hh"

#include <boost/format.hpp>

using boost::format;

SERVICE_REGISTER(iofwd::RPCServer, rpcserver);

namespace iofwd
{
   //========================================================================

   RPCServer::RPCServer (service::ServiceManager & m)
      : service::Service (m),
        log_service_ (lookupService<Log>("log")),
        net_service_ (lookupService<Net>("net")),
        registry_ (new rpc::RPCRegistry ()),
        rpcserver_ (new rpc::RPCServer (*registry_)),
        log_ (log_service_->getSource ("rpc"))
   {
      ZLOG_DEBUG (log_, "Installing net accepthandler...");
      net_service_->getNet()->setAcceptHandler (rpcserver_->getHandler ());
   }

   void RPCServer::registerRPC (const std::string & s,
         const rpc::RPCHandler & h)
   {
      rpc::RPCKey k = rpc::getRPCKey (s);
      ZLOG_DEBUG (log_, format("RPC '%s' -> '%lu'") % s % k);
      registerRPC (k, h);
   }

   void RPCServer::unregisterRPC (const std::string & s)
   {
      unregisterRPC (rpc::getRPCKey (s));
   }

   void RPCServer::registerRPC (const rpc::RPCKey & key, const
         rpc::RPCHandler & handler)
   {
      ZLOG_DEBUG (log_, format("Register RPC key %lu") % key);
      registry_->registerFunction (key, handler);
   }

   void RPCServer::unregisterRPC (const rpc::RPCKey & key)
   {
      ZLOG_DEBUG (log_, format("Unregister RPC key %lu") % key);
      registry_->unregisterFunction (key);
   }


   RPCServer::~RPCServer ()
   {
      ZLOG_DEBUG (log_, "Clearing net accepthandler...");
      net_service_->getNet()->clearAcceptHandler ();
   }

   //========================================================================
}
