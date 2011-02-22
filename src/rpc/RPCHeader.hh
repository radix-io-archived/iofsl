#ifndef RPC_RPCHEADER_HH
#define RPC_RPCHEADER_HH


#include "RPCKey.hh"

namespace rpc
{
   //========================================================================

   /**
    * This header is added to an outgoing RPC connection
    */
   struct RPCHeader
   {
      RPCKey key;
   };

   // Ser/Deser function
   template <typename T>
   void process (T & tr, RPCHeader & h)
   {
      process (tr, h.key);
   }

   /**
    * This header is sent back with the RPC response
    */
   struct RPCResponse
   {
      enum {
         RPC_OK = 0,
         RPC_UNKNOWN
         // RPC_NO_PERM
      };

      uint32_t status;
   };

   template <typename T>
   void process (T & p, RPCResponse & r)
   {
      process (p, r.status);
   }

   //========================================================================
}

#endif
