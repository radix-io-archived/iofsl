#include "IOFWDCreateRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDCreateRequest::ReqParam & IOFWDCreateRequest::decodeParam ()
{
   decodeFileSpec (info_);
   process (req_reader_, attr_);
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
   param_.attr = &attr_;
   return param_;
}

void IOFWDCreateRequest::reply (const CBType & cb,
      const zoidfs::zoidfs_handle_t * handle, int created)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   ASSERT (getReturnCode () != zoidfs::ZFS_OK || handle);
   return simpleOptReply (cb, getReturnCode (), TSSTART << *handle << created);
}

IOFWDCreateRequest::~IOFWDCreateRequest ()
{
    if(op_hint_)
    {
        zoidfs::util::ZoidFSHintDestroy(&op_hint_);
    }
}


//===========================================================================
   }
}
