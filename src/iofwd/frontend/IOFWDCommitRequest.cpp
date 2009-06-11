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
   zoidfs::zoidfs_handle_t handle;
   process (req_reader_, handle);
   param_.handle = &handle;
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDCommitRequest::reply ()
{
   return simpleReply (TSSTART << (int32_t) getReturnCode ()); 
}

//===========================================================================
   }
}
