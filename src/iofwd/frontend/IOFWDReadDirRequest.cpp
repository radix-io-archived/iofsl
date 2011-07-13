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
   decodeOpHint (op_hint_());

   entries_ = new zoidfs::zoidfs_dirent_t[entry_count_];
   memset(entries_, 0, sizeof(zoidfs::zoidfs_dirent_t) * entry_count_);

   /*
    * Init param_ with the decoded XDR data
    */
   param_.handle = &handle_;
   param_.cookie = cookie_;
   param_.entry_count = entry_count_;
   param_.entries = entries_;
   param_.flags = flags_;
   param_.op_hint = &op_hint_;
 
   return param_;
}

void IOFWDReadDirRequest::reply (const CBType & cb,
                                 uint32_t entry_count,
                                 zoidfs::zoidfs_dirent_t * entries,
                                 zoidfs::zoidfs_cache_hint_t * parent_hint)
{
   // If success, send the return code followed by the handle;
   // Otherwise send the return code.
   ASSERT ((getReturnCode() != zoidfs::ZFS_OK) ||
         (entries && parent_hint));
   simpleOptReply (cb, getReturnCode (), TSSTART << entry_count
                          << encoder::EncVarArray(entries, entry_count)
                          << *parent_hint);
}

//===========================================================================
   }
}
