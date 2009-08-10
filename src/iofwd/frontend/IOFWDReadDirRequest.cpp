#include "IOFWDReadDirRequest.hh"
#include "iofwdutil/tools.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

IOFWDReadDirRequest::~IOFWDReadDirRequest()
{
  if (entries_)
    delete[] entries_;
}
   

const IOFWDReadDirRequest::ReqParam & IOFWDReadDirRequest::decodeParam ()
{
   process (req_reader_, handle_);
   process (req_reader_, cookie_);
   process (req_reader_, entry_count_);
   process (req_reader_, flags_);

   entries_ = new zoidfs::zoidfs_dirent_t[entry_count_];

   /*
    * Init param_ with the decoded XDR data
    */
   param_.handle = &handle_;
   param_.cookie = cookie_;
   param_.entry_count = entry_count_;
   param_.entries = entries_;
   param_.flags = flags_;
   
   return param_;
}

iofwdutil::completion::CompletionID * IOFWDReadDirRequest::reply (uint32_t entry_count,
                                                                  zoidfs::zoidfs_dirent_t * entries,
                                                                  zoidfs::zoidfs_cache_hint_t * parent_hint)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   if (getReturnCode() == zoidfs::ZFS_OK)
   {
      ASSERT (entries);
      ASSERT (parent_hint);
      return simpleReply (TSSTART << (int32_t) getReturnCode() << entry_count
                          << iofwdutil::xdr::XDRVarArray(entries, entry_count)
                          << *parent_hint);
   }
   else
   {
      return simpleReply (TSSTART << (int32_t) getReturnCode());
   }
}

//===========================================================================
   }
}
