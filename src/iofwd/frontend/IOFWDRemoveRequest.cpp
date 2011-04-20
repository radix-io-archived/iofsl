#include "IOFWDRemoveRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDRemoveRequest::ReqParam & IOFWDRemoveRequest::decodeParam ()
{
   decodeFileSpec (info_);
   decodeOpHint (op_hint_());
   if (info_.full_path[0])
   {
      param_.full_path = info_.full_path;
      param_.component_name = 0;
      param_.parent_handle = 0;
   }
   else
   {
      param_.full_path = 0;
      param_.parent_handle = &info_.parent_handle ;
      param_.component_name = info_.component_name;
   }
   param_.op_hint = &op_hint_;
   return param_;
}

void IOFWDRemoveRequest::reply (const CBType & cb, const zoidfs::zoidfs_cache_hint_t * parent_hint)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   ASSERT (getReturnCode () != zoidfs::ZFS_OK || parent_hint);
   return simpleOptReply (cb, getReturnCode (), TSSTART << *parent_hint);
}

IOFWDRemoveRequest::~IOFWDRemoveRequest ()
{
}


//===========================================================================
   }
}
