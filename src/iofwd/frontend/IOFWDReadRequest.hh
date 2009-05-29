#ifndef IOFWD_FRONTEND_IOFWDREADREQUEST_HH
#define IOFWD_FRONTEND_IOFWDREADREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/ReadRequest.hh"

namespace iofwd
{
   namespace frontend
   {

class  IOFWDReadRequest 
   : public IOFWDRequest, 
     public ReadRequest
{
public:
   IOFWDReadRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info)
      : IOFWDRequest (bmi, info), ReadRequest (opid)
   {
   }
       
   virtual void returnData (const void * buf[], const size_t size[],
             int count) ; 

   virtual ~IOFWDReadRequest (); 
}; 



   }
}

#endif
