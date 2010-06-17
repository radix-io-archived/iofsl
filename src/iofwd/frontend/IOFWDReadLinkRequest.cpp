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
   decodeOpHint (&op_hint_);

   /*
    * Init param_ with the decoded XDR data
    */
   param_.buffer_length = buffer_length_;
   param_.handle = &handle_;
   if(op_hint_)
   {
      param_.op_hint = op_hint_;
   }
   else
   {
      param_.op_hint = NULL;
   }

   return param_;
}

void IOFWDReadLinkRequest::reply (const CBType & cb, const char * buffer, size_t buffer_length)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.

   ASSERT (getReturnCode() != zoidfs::ZFS_OK || buffer);
   simpleOptReply (cb, getReturnCode (),
         TSSTART << encoder::EncString(buffer, buffer_length));
}

IOFWDReadLinkRequest::~IOFWDReadLinkRequest ()
{
    if(op_hint_)
    {
        zoidfs::util::ZoidFSHintDestroy(&op_hint_);
    }
}


//===========================================================================
   }
}
