#ifndef IOFWD_FRONTENT_IOFWDNOTIMPLEMENTEDREQUEST_HH
#define IOFWD_FRONTENT_IOFWDNOTIMPLEMENTEDREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/NotImplementedRequest.hh"

namespace iofwd
{
   namespace frontend
   {

class IOFWDNotImplementedRequest : public NotImplementedRequest,
                                   public IOFWDRequest
{
public:
   IOFWDNotImplementedRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info)
      : NotImplementedRequest(opid), IOFWDRequest(bmi, info)
   {
   }

 virtual bool run ()
   { return IOFWDRequest::run (); }

   virtual ~IOFWDNotImplementedRequest (); 


}; 

   }
}

#endif
