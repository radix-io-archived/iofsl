#ifndef IOFWD_NULLREQUEST_HH
#define IOFWD_NULLREQUEST_HH

#include "Request.hh"
#include "iofwdutil/completion/CompletionID.hh"

// #include "NullRequestSM.hh"

namespace iofwd
{

class NullRequest : public Request
{
public:
   NullRequest (int opid)
      : Request (opid)
   {
   }

   virtual void reply (const CBType & cb) = 0;
};

}

#endif
