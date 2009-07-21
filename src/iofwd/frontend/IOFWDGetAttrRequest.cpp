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
   param_.handle = &handle_;
   param_.attr = &attr_;
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDGetAttrRequest::reply (const zoidfs::zoidfs_attr_t * attr)
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

IOFWDGetAttrRequest::~IOFWDGetAttrRequest ()
{
}

//===========================================================================
   }
}
