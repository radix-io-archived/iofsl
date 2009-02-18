#ifndef IOFWD_FRONTEND_IOFWDREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREQUEST_HH

#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/bmi/BMIAddr.hh"
#include "iofwdutil/bmi/BMITag.hh"
#include "iofwdutil/bmi/BMIUnexpectedBuffer.hh"

namespace iofwd
{
   namespace frontend
   {

class IOFWDRequest 
{
public:
   IOFWDRequest (const BMI_unexpected_info & info); 


   /// Release the memory of the incoming request
   void freeRawRequest (); 

   virtual ~IOFWDRequest (); 

   virtual bool run ()
   {
      return false; 
   }

protected:
   // Memory holding the request
   iofwdutil::bmi::BMIUnexpectedBuffer raw_request_; 

   // Where our client is located
   iofwdutil::bmi::BMIAddr addr_;
   iofwdutil::bmi::BMITag  tag_; 

}; 


   }
}

#endif
