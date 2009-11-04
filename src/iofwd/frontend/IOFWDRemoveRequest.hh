#ifndef IOFWD_FRONTEND_IOFWDREMOVEREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREMOVEREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/RemoveRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDRemoveRequest 
   : public IOFWDRequest, 
     public RemoveRequest
{
public:
   IOFWDRemoveRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info,res), RemoveRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ; 

   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_cache_hint_t * parent_hint);

   virtual ~IOFWDRemoveRequest (); 

   
protected:
   ReqParam param_; 
   FileInfo info_;
   zoidfs::zoidfs_op_hint_t * op_hint_;
}; 

//===========================================================================
   }
}

#endif
