#include "IOFWDNullRequest.hh"
#include "zoidfs/zoidfs-wrapped.hh"
#include "iofwdutil/xdr/XDRSizeProcessor.hh"

using namespace iofwdutil::xdr; 


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

void IOFWDNullRequest::reply ()
{
   setReturnCode (zoidfs::ZFS_OK); 
   beginReply (getXDRSize<int32_t>().max ); 
   reply_writer_ << (int32_t) getReturnCode (); 
   sendReply (); 
}

IOFWDNullRequest::~IOFWDNullRequest ()
{
}

//===========================================================================
   }
}
