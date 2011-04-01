#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

IOFSLRPCRequest::IOFSLRPCRequest(iofwdevent::ZeroCopyInputStream * in,
        iofwdevent::ZeroCopyOutputStream * out) :
    in_(in),
    out_(out)
{
}

IOFSLRPCRequest::~IOFSLRPCRequest ()
{
}

   }
}
