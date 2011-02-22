#ifndef RPC_RPCHANDLER_HH
#define RPC_RPCHANDLER_HH

#include "iofwdevent/ZeroCopyInputStream.hh"
#include "iofwdevent/ZeroCopyOutputStream.hh"

namespace rpc
{
   //========================================================================

   class RPCInfo;

   /**
    * RPC (server side) handler function
    *    - Takes a ZeroCopy input and output stream.
    *    - The function should not block, and destroy both streams when done.
    */

   typedef boost::function<void (
             iofwdevent::ZeroCopyInputStream  *,
             iofwdevent::ZeroCopyOutputStream *,
             const RPCInfo & info
            )>   RPCHandler;

   //========================================================================
}

#endif
