#ifndef IOFWD_FRONTEND_IOFWDNULLREQUEST_HH
#define IFOWD_FRONTEND_IOFWDNULLREQUEST_HH

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
   IOFWDNullRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info)
      : NullRequest(opid), IOFWDRequest (bmi, info)
   {
   }

   virtual void reply ();

   virtual ~IOFWDNullRequest (); 
};


//===========================================================================
   }
}


#endif
