#ifndef IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH
#define IOFWD_FRONTEND_IOFWDLOOKUPREQUEST_HH

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

   virtual bool run ()
   { return IOFWDRequest::run(); }
   
   virtual void reply (const zoidfs_handle_t * handle); 

   virtual ~IOFWDLookupRequest (); 
}; 

//===========================================================================
   }
}

#endif
