#ifndef ROSS_ROSSFRONTEND_ROSSNULLREQUEST_HH
#define ROSS_ROSSFRONTEND_ROSSNULLREQUEST_HH

#include "iofwd/NullRequest.hh"
#include "iofwd/rossfrontend/ROSSRequest.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{
   namespace rossfrontend
   {

class ROSSNullRequest : public NullRequest,
                         public ROSSRequest
{
public:
   ROSSNullRequest(int opid)
      : NullRequest(opid), ROSSRequest()
   {
   }

   virtual void reply(const CBType & cb);
};

   }
}


#endif
