#include "IOFWDSetAttrRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDSetAttrRequest::ReqParam & IOFWDSetAttrRequest::decodeParam ()
{
   process (req_reader_, handle_);
   process (req_reader_, sattr_);
   process (req_reader_, attr_);
   decodeOpHint (&op_hint_);
   param_.handle = &handle_;
   param_.sattr = &sattr_;
   param_.attr = &attr_;
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

void IOFWDSetAttrRequest::reply (const CBType & cb, const zoidfs::zoidfs_attr_t * attr)
{
   // If success, send the return code followed by the attr;
   // Otherwise send the return code.
   ASSERT (getReturnCode() != zoidfs::ZFS_OK || attr);
   simpleOptReply (cb, getReturnCode(), TSSTART << *attr);
}

IOFWDSetAttrRequest::~IOFWDSetAttrRequest ()
{
    if(op_hint_)
    {
        zoidfs::util::ZoidFSHintDestroy(&op_hint_);
    }
}

//===========================================================================
   }
}
