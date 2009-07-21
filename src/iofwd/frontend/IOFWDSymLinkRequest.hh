#ifndef IOFWD_FRONTEND_IOFWDSYMLINKREQUEST_HH
#define IOFWD_FRONTEND_IOFWDSYMLINKREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/SymLinkRequest.hh"


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDSymLinkRequest
   : public IOFWDRequest,
     public SymLinkRequest
{
public:
   IOFWDSymLinkRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info,res), SymLinkRequest (opid)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_cache_hint_t * from_parent_hint,
                                                        const zoidfs::zoidfs_cache_hint_t * to_parent_hint);

   virtual ~IOFWDSymLinkRequest ();

protected:
   ReqParam param_;
   FileInfo from_info_;
   FileInfo to_info_;
   zoidfs::zoidfs_sattr_t sattr_;
};

//===========================================================================
   }
}

#endif
