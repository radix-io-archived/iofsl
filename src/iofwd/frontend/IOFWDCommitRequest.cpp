#include "IOFWDCommitRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDCommitRequest::~IOFWDCommitRequest ()
{
    if(op_hint_)
    {
        zoidfs::util::ZoidFSHintDestroy(&op_hint_);
    }
}

const IOFWDCommitRequest::ReqParam & IOFWDCommitRequest::decodeParam ()
{
   process (req_reader_, handle_);
   decodeOpHint (&op_hint_);
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

iofwdutil::completion::CompletionID * IOFWDCommitRequest::reply ()
{
   return simpleReply (TSSTART << (int32_t) getReturnCode ()); 
}

//===========================================================================
   }
}
