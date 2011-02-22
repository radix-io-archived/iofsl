#ifndef RPC_RPCKEY_HH
#define RPC_RPCKEY_HH

#include <string>
#include <stdint.h>

namespace rpc
{
   //========================================================================

   typedef uint64_t RPCKey;

   RPCKey getRPCKey (const std::string & s);
   RPCKey getRPCKey (const char * s);

   //========================================================================
}

#endif
