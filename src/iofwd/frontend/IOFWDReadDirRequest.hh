#ifndef IOFWD_FRONTEND_IOFWDREADDIRREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/ReadDirRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDReadDirRequest
   : public IOFWDRequest,
     public ReadDirRequest
{
public:
   IOFWDReadDirRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info,res), ReadDirRequest (opid), entries_ (NULL)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual iofwdutil::completion::CompletionID * reply (uint32_t entry_count,
                                                        zoidfs::zoidfs_dirent_t * entries,
                                                        zoidfs::zoidfs_cache_hint_t * cache);

   virtual ~IOFWDReadDirRequest ();

protected:
   ReqParam param_;

   zoidfs::zoidfs_handle_t handle_;
   zoidfs::zoidfs_dirent_cookie_t cookie_;
   uint32_t entry_count_;
   zoidfs::zoidfs_dirent_t * entries_;
   uint32_t flags_;
};

//===========================================================================
   }
}

#endif
