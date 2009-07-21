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

   return param_;
}

iofwdutil::completion::CompletionID * IOFWDRenameRequest::reply (const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                                                                 const zoidfs::zoidfs_cache_hint_t * to_parent_hint)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   if (getReturnCode() == zoidfs::ZFS_OK)
   {
      ASSERT (from_parent_hint);
      ASSERT (to_parent_hint);
      return simpleReply (TSSTART << (int32_t) getReturnCode() << *from_parent_hint << *to_parent_hint);
   }
   else
   {
      return simpleReply (TSSTART << (int32_t) getReturnCode());
   }
}

IOFWDRenameRequest::~IOFWDRenameRequest ()
{
}


//===========================================================================
   }
}
