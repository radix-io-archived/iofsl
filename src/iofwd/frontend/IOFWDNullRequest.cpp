#include "IOFWDNullRequest.hh"
#include "zoidfs/util/zoidfs-wrapped.hh"
#include "encoder/xdr/XDRSizeProcessor.hh"

using namespace encoder::xdr;


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

void IOFWDNullRequest::reply (const CBType & cb)
{
}

iofwdutil::completion::CompletionID * IOFWDNullRequest::reply ()
{
   return simpleReply (TSSTART << (int32_t) getReturnCode ());
}

//===========================================================================
   }
}
