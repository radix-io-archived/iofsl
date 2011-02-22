#include "RPCRegistry.hh"
#include "RPCException.hh"

#include <algorithm>

namespace rpc
{
   //========================================================================

   void RPCRegistry::registerFunction (const RPCKey & key, const RPCHandler &
         handler)
   {
      boost::mutex::scoped_lock l(lock_);
      registry_.insert (std::make_pair (key, handler));
   }

   void RPCRegistry::unregisterFunction (const RPCKey & key)
   {
      boost::mutex::scoped_lock l(lock_);
      if (!registry_.erase (key))
         ZTHROW (UnknownRPCKeyException ());
   }

   const RPCHandler & RPCRegistry::lookupFunction (const RPCKey & key) const
   {
      boost::mutex::scoped_lock l(lock_);
      MapType::const_iterator I = registry_.find (key);
      if (I == registry_.end ())
         ZTHROW (UnknownRPCKeyException ());
      return I->second;
   }

   //========================================================================
}
