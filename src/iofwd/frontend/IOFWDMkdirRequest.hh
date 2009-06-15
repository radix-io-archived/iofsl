#ifndef IOFWD_FRONTEND_IOFWDMKDIRREQUEST_HH
#define IOFWD_FRONTEND_IOFWDMKDIRREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/MkdirRequest.hh"


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDMkdirRequest
   : public IOFWDRequest,
     public MkdirRequest
{
public:
   IOFWDMkdirRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info,res), MkdirRequest (opid)
   {
   }

   virtual const ReqParam & decodeParam () ;

   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_cache_hint_t * parent_hint);

   virtual ~IOFWDMkdirRequest ();


protected:
   ReqParam param_;
   FileInfo info_;
   zoidfs::zoidfs_sattr_t sattr_;
};

//===========================================================================
   }
}

#endif
