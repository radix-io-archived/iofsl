#ifndef RPC_RPCREGISTRY_HH
#define RPC_RPCREGISTRY_HH

#include "RPCKey.hh"
#include "iofwdutil/ZException.hh"
#include "RPCHandler.hh"

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

namespace rpc
{
   //========================================================================


   struct RPCRegistry
   {
      public:

         void registerFunction (const RPCKey & key, const RPCHandler &
               handler);

         const RPCHandler &  lookupFunction (const RPCKey & key) const;

         void unregisterFunction (const RPCKey & key);

      protected:
         mutable boost::mutex lock_;

         typedef boost::unordered_map<RPCKey, RPCHandler> MapType;
         MapType registry_;
   };

   //========================================================================
};

#endif
