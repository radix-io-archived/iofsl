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

/*   virtual  run ()
   { 
      reqstatus r = IOFWDRequest::run (); 
      beginReply (128); 
      static int i = 0; 
      ++i; 
      setStatus (i); 
      reply_writer_ << status_; 
      sendReply ().wait(); 
      return r; 
   } */

   virtual ~IOFWDNullRequest (); 
};


//===========================================================================
   }
}


#endif
