#ifndef IOFWD_RPCEXCEPTION_HH
#define IOFWD_RPCEXCEPTION_HH

#include "iofwdutil/ZException.hh"
#include "rpc/RPCKey.hh"

namespace iofwd
{
   //========================================================================

   /*
    * RPC related exceptions
    */
   struct RPCException : virtual public iofwdutil::ZException {};

   /// Thrown when an invalid RPC key was specified
   struct InvalidRPCKeyException : virtual public RPCException {};

   typedef boost::error_info<struct tag_rpckey,rpc::RPCKey> exception_rpckey;


   //========================================================================
}

#endif
