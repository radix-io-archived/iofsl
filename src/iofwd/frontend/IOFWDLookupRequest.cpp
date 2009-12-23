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

void IOFWDLookupRequest::reply (const CBType & cb, const zoidfs::zoidfs_handle_t * handle)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   ASSERT (getReturnCode() != zoidfs::ZFS_OK || handle);
   simpleOptReply (cb, getReturnCode (), TSSTART << *handle);
}

IOFWDLookupRequest::~IOFWDLookupRequest ()
{
    if(op_hint_)
    {
        zoidfs::util::ZoidFSHintDestroy(&op_hint_);
    }
}


//===========================================================================
   }
}
