#include "IOFWDNullRequest.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "iofwdutil/xdr/XDRSizeProcessor.hh"

using namespace iofwdutil::xdr; 


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

void IOFWDNullRequest::reply ()
{
   simpleReply (TSSTART << (int32_t) getReturnCode ()); 
}

IOFWDNullRequest::~IOFWDNullRequest ()
{
}

//===========================================================================
   }
}
