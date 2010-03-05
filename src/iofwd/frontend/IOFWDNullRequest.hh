#ifndef IOFWD_FRONTEND_IOFWDNULLREQUEST_HH
#define IOFWD_FRONTEND_IOFWDNULLREQUEST_HH

#include "iofwd/NullRequest.hh"
#include "iofwd/frontend/IOFWDRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace frontend
   {
//===========================================================================

class IOFWDNullRequest : public NullRequest,
                         public IOFWDRequest,
                         public iofwdutil::InjectPool<IOFWDNullRequest>
{
public:
   IOFWDNullRequest (int opid, const BMI_unexpected_info & info,
         IOFWDResources & res)
      : NullRequest(opid), IOFWDRequest (info, res)
   {
   }

   virtual void reply (const CBType & cb);
};


//===========================================================================
   }
}


#endif
