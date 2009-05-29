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
   IOFWDLookupRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info)
      : IOFWDRequest (bmi, info), LookupRequest (opid)
   {
   }

   virtual const ReqParam & decodeParam () ; 

   virtual void reply (const zoidfs::zoidfs_handle_t * handle); 

   virtual ~IOFWDLookupRequest (); 

   
protected:
   ReqParam param_; 
   FileInfo info_; 
}; 

//===========================================================================
   }
}

#endif
