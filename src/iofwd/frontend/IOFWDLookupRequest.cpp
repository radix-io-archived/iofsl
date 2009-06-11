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

iofwdutil::completion::CompletionID * IOFWDLookupRequest::reply (const zoidfs::zoidfs_handle_t * handle)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   if (getReturnCode() == zoidfs::ZFS_OK)
   {
      ASSERT (handle); 
      return simpleReply (TSSTART << (int32_t) getReturnCode() << *handle); 
   }
   else
   {
      return simpleReply (TSSTART << (int32_t) getReturnCode()); 
   }
}

IOFWDLookupRequest::~IOFWDLookupRequest ()
{
}


//===========================================================================
   }
}
