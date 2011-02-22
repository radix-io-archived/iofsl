#include "iofwd/RPCClient.hh"

#include "iofwd/Log.hh"
#include "iofwd/Net.hh"

#include "rpc/RPCKey.hh"
#include "rpc/RPCClient.hh"

SERVICE_REGISTER(iofwd::RPCClient, rpcclient);

namespace iofwd
{
   //========================================================================

   RPCClient::RPCClient (service::ServiceManager & m)
      : service::Service (m),
        log_service_ (lookupService<Log> ("log")),
        net_service_ (lookupService<Net> ("net"))
   {
   }

   rpc::RPCClientHandle RPCClient::rpcConnect (const char * s, const net::AddressPtr & addr)
   {
      return rpcConnect (rpc::getRPCKey (s), addr);
   }

   rpc::RPCClientHandle RPCClient::rpcConnect (const rpc::RPCKey & k, const net::AddressPtr & addr)
   {
      net::Connection con = net_service_->getNet()->connect (addr);
      return rpc::RPCClient::rpcConnect (k, con);
   }


   RPCClient::~RPCClient ()
   {
   }

   //========================================================================
}
