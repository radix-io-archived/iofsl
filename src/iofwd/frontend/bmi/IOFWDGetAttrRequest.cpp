#include "IOFWDGetAttrRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDGetAttrRequest::ReqParam & IOFWDGetAttrRequest::decodeParam ()
{
   process (req_reader_, handle_);
   process (req_reader_, attr_);
   zoidfs::hints::zoidfs_hint_create(&op_hint_);
   decodeOpHint (&op_hint_);
   param_.handle = &handle_;
   param_.attr = &attr_;
   param_.op_hint = &op_hint_;
   return param_;
}

void IOFWDGetAttrRequest::reply (const CBType & cb, const zoidfs::zoidfs_attr_t * attr)
{
   // If success, send the return code followed by the attr;
   // Otherwise send the return code.
   ASSERT (getReturnCode () != zoidfs::ZFS_OK || attr);
   simpleOptReply (cb, getReturnCode (), TSSTART << *attr);
}

IOFWDGetAttrRequest::~IOFWDGetAttrRequest ()
{
   zoidfs::hints::zoidfs_hint_free(&op_hint_);
}

//===========================================================================
   }
}
