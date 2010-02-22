#ifndef IOFWD_FRONTEND_IOFWDNULLREQUEST_HH
#define IOFWD_FRONTEND_IOFWDNULLREQUEST_HH

#include "iofwd/NullRequest.hh"
#include "iofwd/frontend/IOFWDRequest.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDNullRequest : public NullRequest,
                         public IOFWDRequest
{
public:
   IOFWDNullRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : NullRequest(opid), IOFWDRequest (bmi, info, res)
   {
   }

   virtual iofwdutil::completion::CompletionID * reply ();

   virtual ~IOFWDNullRequest (); 
};


//===========================================================================
   }
}


#endif
