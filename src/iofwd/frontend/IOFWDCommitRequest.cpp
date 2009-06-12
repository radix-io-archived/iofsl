#include "IOFWDCommitRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDCommitRequest::~IOFWDCommitRequest ()
{
}

const IOFWDCommitRequest::ReqParam & IOFWDCommitRequest::decodeParam ()
{
   process (req_reader_, handle_);
   param_.handle = &handle_;
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDCommitRequest::reply ()
{
   return simpleReply (TSSTART << (int32_t) getReturnCode ()); 
}

//===========================================================================
   }
}
