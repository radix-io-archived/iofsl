#ifndef RPC_RPCEXCEPTION_HH
#define RPC_RPCEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace rpc
{
   //========================================================================

   struct RPCException : virtual iofwdutil::ZException {};

   struct RPCCommException : virtual RPCException {};

   struct UnknownRPCKeyException : virtual RPCException {};

   //========================================================================
}
#endif
