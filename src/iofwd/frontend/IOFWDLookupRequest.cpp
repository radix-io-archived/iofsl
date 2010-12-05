#include "IOFWDLookupRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDLookupRequest::ReqParam & IOFWDLookupRequest::decodeParam ()
{
   decodeFileSpec (info_);
   zoidfs::hints::zoidfs_hint_create(&op_hint_);
   decodeOpHint (&op_hint_);
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

void IOFWDLookupRequest::reply (const CBType & cb, const zoidfs::zoidfs_handle_t * handle)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   ASSERT (getReturnCode() != zoidfs::ZFS_OK || handle);
   simpleOptReply (cb, getReturnCode (), TSSTART << *handle);
}

IOFWDLookupRequest::~IOFWDLookupRequest ()
{
   zoidfs::hints::zoidfs_hint_free(&op_hint_);
}


//===========================================================================
   }
}
