#include "IOFWDMkdirRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

const IOFWDMkdirRequest::ReqParam & IOFWDMkdirRequest::decodeParam () 
{
   decodeFileSpec (info_);
   process (req_reader_, sattr_);
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
   param_.sattr = &sattr_;
   return param_; 
}

iofwdutil::completion::CompletionID * IOFWDMkdirRequest::reply (const zoidfs::zoidfs_cache_hint_t * parent_hint)
{
   // If success, send the return code followed by the hint;
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

IOFWDMkdirRequest::~IOFWDMkdirRequest ()
{
}


//===========================================================================
   }
}
