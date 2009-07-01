#include "IOFWDResizeRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDResizeRequest::~IOFWDResizeRequest ()
{
}

const IOFWDResizeRequest::ReqParam & IOFWDResizeRequest::decodeParam ()
{
   process (req_reader_, handle_);
   process (req_reader_, size_);
   param_.handle = &handle_;
   param_.size = size_;
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDResizeRequest::reply ()
{
   return simpleReply (TSSTART << (int32_t) getReturnCode ()); 
}

//===========================================================================
   }
}
