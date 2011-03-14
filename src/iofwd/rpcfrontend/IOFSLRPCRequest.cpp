#include "iofwd/rpcfrontend/IOFSLRPCRequest.hh"

namespace iofwd
{
   namespace rpcfrontend
   {

IOFSLRPCRequest::IOFSLRPCRequest(iofwdevent::ZeroCopyInputStream * in,
        iofwdevent::ZeroCopyOutputStream * out) :
    in_(in),
    out_(out),
    read_ptr_(NULL),
    read_size_(0),
    write_ptr_(NULL),
    write_size_(0),
    insize_(0),
    outsize_(0)
{
}

IOFSLRPCRequest::~IOFSLRPCRequest ()
{
}

   }
}
