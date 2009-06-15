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
   return param_; 
}

iofwdutil::completion::CompletionID * IOFWDRemoveRequest::reply (const zoidfs::zoidfs_cache_hint_t * parent_hint)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   if (getReturnCode() == zoidfs::ZFS_OK)
   {
      ASSERT (parent_hint); 
      return simpleReply (TSSTART << (int32_t) getReturnCode() << *parent_hint);
   }
   else
   {
      return simpleReply (TSSTART << (int32_t) getReturnCode()); 
   }
}

IOFWDRemoveRequest::~IOFWDRemoveRequest ()
{
}


//===========================================================================
   }
}
