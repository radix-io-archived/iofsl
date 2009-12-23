#include "IOFWDResizeRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDResizeRequest::~IOFWDResizeRequest ()
{
    if(op_hint_)
    {
        zoidfs::util::ZoidFSHintDestroy(&op_hint_);
    }
}

const IOFWDResizeRequest::ReqParam & IOFWDResizeRequest::decodeParam ()
{
   process (req_reader_, handle_);
   process (req_reader_, size_);
   decodeOpHint (&op_hint_);
   param_.handle = &handle_;
   param_.size = size_;
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

void IOFWDResizeRequest::reply (const CBType & cb)
{
   simpleReply (cb, TSSTART << (int32_t) getReturnCode ());
}

//===========================================================================
   }
}
