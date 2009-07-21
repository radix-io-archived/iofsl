#include "IOFWDReadLinkRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDReadLinkRequest::ReqParam & IOFWDReadLinkRequest::decodeParam ()
{
   process (req_reader_, handle_);
   process (req_reader_, buffer_length_);
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDReadLinkRequest::reply (const char * buffer, uint64_t buffer_length)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   if (getReturnCode() == zoidfs::ZFS_OK)
   {
      ASSERT (buffer);
      return simpleReply (TSSTART << (int32_t) getReturnCode() << iofwdutil::xdr::XDRString(buffer, buffer_length));
   }
   else
   {
      return simpleReply (TSSTART << (int32_t) getReturnCode());
   }
}

IOFWDReadLinkRequest::~IOFWDReadLinkRequest ()
{
}


//===========================================================================
   }
}
