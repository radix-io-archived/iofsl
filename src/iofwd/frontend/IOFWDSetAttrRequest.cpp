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
   decodeOpHint (op_hint_());
   param_.handle = &handle_;
   param_.sattr = &sattr_;
   param_.attr = &attr_;
   param_.op_hint = &op_hint_;
   return param_;
}

void IOFWDSetAttrRequest::reply (const CBType & cb,
      const zoidfs::zoidfs_attr_t * attr)
{
   // If success, send the return code followed by the attr;
   // Otherwise send the return code.
   ASSERT (getReturnCode() != zoidfs::ZFS_OK || attr);
   
   //zoidfs::hints::zoidfs_hint_delete_all(*param_.op_hint);
   //zoidfs::hints::zoidfs_hint_set ((*param_.op_hint), ZOIDFS_SFP_VAL,
   //      "99", 3);
   simpleOptReply (cb, getReturnCode(), TSSTART << *attr
           << *((*param_.op_hint)()));
}

IOFWDSetAttrRequest::~IOFWDSetAttrRequest ()
{
}

//===========================================================================
   }
}
