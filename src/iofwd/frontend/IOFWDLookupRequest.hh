#ifndef IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH
#define IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH

#include "zoidfs/util/zoidfs-wrapped.hh"
#include "IOFWDRequest.hh"
#include "iofwd/LookupRequest.hh"


namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDLookupRequest 
   : public IOFWDRequest, 
     public LookupRequest
{
public:
   IOFWDLookupRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : IOFWDRequest (bmi, info,res), LookupRequest (opid), op_hint_(NULL)
   {
   }

   virtual const ReqParam & decodeParam () ; 

   virtual iofwdutil::completion::CompletionID * reply (const zoidfs::zoidfs_handle_t * handle); 

   virtual ~IOFWDLookupRequest (); 

   
protected:
   ReqParam param_; 
   FileInfo info_;
   zoidfs::zoidfs_op_hint_t * op_hint_; 
}; 

//===========================================================================
   }
}

#endif
