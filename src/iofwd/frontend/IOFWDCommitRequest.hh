#ifndef IOFWD_FRONTEND_IOFWDCOMMITREQUEST_HH
#define IOFWD_FRONTEND_IOFWDCOMMITREQUEST_HH

#include "IOFWDRequest.hh"
#include "iofwd/CommitRequest.hh"

namespace iofwd
{

   namespace frontend
   {
//===========================================================================

class IOFWDCommitRequest : public CommitRequest,
                         public IOFWDRequest
{
public:
   IOFWDCommitRequest (iofwdutil::bmi::BMIContext & bmi, int opid, const BMI_unexpected_info & info,
         iofwdutil::completion::BMIResource & res)
      : CommitRequest(opid), IOFWDRequest (bmi, info, res)
   {
   }

   virtual CompletionID * reply ();

   virtual ~IOFWDCommitRequest (); 
};


//===========================================================================
   }

}

#endif
