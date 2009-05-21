#ifndef IOFWD_NULLREQUEST_HH
#define IOFWD_NULLREQUEST_HH

#include "Request.hh"
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

   virtual void reply () = 0; 

   virtual ~NullRequest (); 
}; 

}

#endif
