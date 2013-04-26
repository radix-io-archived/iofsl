#include "IOFWDCommitRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

      // @TODO: make op_hint a C++ class so no explicit destruction is needed.
IOFWDCommitRequest::~IOFWDCommitRequest ()
{
    zoidfs::hints::zoidfs_hint_free(&op_hint_);
}

const IOFWDCommitRequest::ReqParam & IOFWDCommitRequest::decodeParam ()
{
   process (req_reader_, handle_);
   zoidfs::hints::zoidfs_hint_create(&op_hint_);
   decodeOpHint (&op_hint_);
   param_.handle = &handle_;
   param_.op_hint = &op_hint_;
   return param_;
}

void IOFWDCommitRequest::reply (const CBType & cb)
{
   simpleReply (cb, TSSTART << (int32_t) getReturnCode ());
}

//===========================================================================
   }
}
