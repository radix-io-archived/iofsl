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
   param_.handle = &handle_;
   param_.sattr = &sattr_;
   param_.attr = &attr_;
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDSetAttrRequest::reply (const zoidfs::zoidfs_attr_t * attr)
{
   // If success, send the return code followed by the attr;
   // Otherwise send the return code.
   if (getReturnCode() == zoidfs::ZFS_OK)
   {
      ASSERT (attr);
      return simpleReply (TSSTART << (int32_t) getReturnCode() << *attr);
   }
   else
   {
      return simpleReply (TSSTART << (int32_t) getReturnCode());
   }
}

IOFWDSetAttrRequest::~IOFWDSetAttrRequest ()
{
}

//===========================================================================
   }
}
