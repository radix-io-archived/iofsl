#include "IOFWDCommitRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

      // @TODO: make op_hint a C++ class so no explicit destruction is needed.
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

void IOFWDCommitRequest::reply (const CBType & cb)
{
   simpleReply (cb, TSSTART << (int32_t) getReturnCode ());
}

//===========================================================================
   }
}
