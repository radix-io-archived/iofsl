#ifndef IOFWD_RPC_HH
#define IOFWD_RPC_HH

#include "iofwd/service/Service.hh"

#include "rpc/RPCKey.hh"
#include "net/Net.hh"
#include "rpc/RPCClient.hh"

namespace iofwd
{
   //========================================================================

   class Net;
   class Log;

   /**
    * High-level RPC Communication service
    *
    * @TODO: Over time, add more high level functionality here...
    */
   struct RPCClient : public service::Service
   {
      public:

         RPCClient (service::ServiceManager & m);

         rpc::RPCClientHandle rpcConnect (const char * s, const net::AddressPtr & addr);

         rpc::RPCClientHandle rpcConnect (const rpc::RPCKey & k, const net::AddressPtr & addr);

         virtual ~RPCClient ();

      protected:
         boost::shared_ptr<Log> log_service_;
         boost::shared_ptr<Net> net_service_;
   };

   //========================================================================
}

#endif
