#include "IOFWDRenameRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDRenameRequest::ReqParam & IOFWDRenameRequest::decodeParam ()
{
   decodeFileSpec (from_info_);
   decodeFileSpec (to_info_);
   decodeOpHint (op_hint_());

   // from
   if (from_info_.full_path[0])
   {
      param_.from_full_path = from_info_.full_path;
      param_.from_component_name = 0;
      param_.from_parent_handle = 0;
   }
   else
   {
      param_.from_full_path = 0;
      param_.from_parent_handle = &from_info_.parent_handle ;
      param_.from_component_name = from_info_.component_name;
   }

   // to
   if (to_info_.full_path[0])
   {
      param_.to_full_path = to_info_.full_path;
      param_.to_component_name = 0;
      param_.to_parent_handle = 0;
   }
   else
   {
      param_.to_full_path = 0;
      param_.to_parent_handle = &to_info_.parent_handle ;
      param_.to_component_name = to_info_.component_name;
   }
   param_.op_hint = &op_hint_;
   return param_;
}

void IOFWDRenameRequest::reply (const CBType & cb,
      const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
     const zoidfs::zoidfs_cache_hint_t * to_parent_hint)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   ASSERT ((getReturnCode() != zoidfs::ZFS_OK) ||
         (from_parent_hint && to_parent_hint));
   simpleOptReply (cb, getReturnCode (), TSSTART << *from_parent_hint << *to_parent_hint);
}

IOFWDRenameRequest::~IOFWDRenameRequest ()
{
   // op_hint_ is of type ZoidFSOpHint;
   // ZoidFSOpHint automatically frees the underlying hint type,
   // so it is illegal to free it manually.
   // zoidfs::hints::zoidfs_hint_free(op_hint_());
}



//===========================================================================
   }
}
