#ifndef RPC_RPCSERVER_HH
#define RPC_RPCSERVER_HH

#include "iofwdutil/IOFWDLog-fwd.hh"
#include "net/Net.hh"
#include "RPCHandler.hh"
#include "RPCKey.hh"

namespace rpc
{
   //========================================================================

   class RPCRegistry;

   /**
    * Takes care of executing incoming RPC calls.
    *
    * Possible extensions: add policy for executing the RPC handler
    * (to be able to switch between using a thread or executing directly)
    */
   class RPCServer
   {
      public:

         RPCServer (RPCRegistry & r);

         /**
          * Return a handler reference suitable for passing to the
          * Net::setAcceptHandler funcion.
          *
          * NOTE: the returned handler becomes invalid when the RPCServer is
          * destroyed!
          */
         net::Net::AcceptHandler getHandler ();

         ~RPCServer ();

      protected:
         class RPCHelper;

         friend class RPCHelper;

         void rpcHandler (const net::Net::AcceptInfo & info);

         const RPCHandler & lookupHandler (const RPCKey & key) const;

      protected:
         RPCRegistry & registry_;
         iofwdutil::IOFWDLogSource & log_;
   };

   //========================================================================
}

#endif
