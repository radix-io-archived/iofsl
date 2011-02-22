#ifndef IOFWD_RPCSERVER_HH
#define IOFWD_RPCSERVER_HH

#include "iofwd/service/Service.hh"

#include "rpc/RPCKey.hh"
#include "rpc/RPCHandler.hh"
#include "rpc/RPC-fwd.hh"


#include <string>

namespace iofwd
{
   //========================================================================

   class Log;
   class Net;

   /**
    * RPC server service.
    *
    *    Supports registration of RPC functions, and takes the necessary steps
    *    to ensure they get called and the response gets sent back.
    */
   class RPCServer : public service::Service
   {
      public:

         RPCServer (service::ServiceManager & m);

         virtual ~RPCServer ();

      public:


         /**
          * Register RPC key key and assign handler as the handler for the
          * key.
          * Throws InvalidRPCKeyException if the key is already registered.
          */
         void registerRPC (const rpc::RPCKey & key,
               const rpc::RPCHandler & handler);

         /**
          * Unregister the specified RPC key
          * Throws InvalidRPCKeyException if the key is unknown.
          */
         void unregisterRPC (const rpc::RPCKey & key);

         /**
          * Convenience register function.
          */
         void registerRPC (const std::string & s, const rpc::RPCHandler & h);

         /**
          * Convenience unregister function.
          */
         void unregisterRPC (const std::string & s);


      protected:
         boost::shared_ptr<Log> log_service_;
         boost::shared_ptr<Net> net_service_;

         boost::scoped_ptr<rpc::RPCRegistry> registry_;
         boost::scoped_ptr<rpc::RPCServer> rpcserver_;

         iofwdutil::IOFWDLogSource & log_;
   };

   //========================================================================
}

#endif
