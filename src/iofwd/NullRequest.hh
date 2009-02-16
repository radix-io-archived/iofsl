#ifndef IOFWD_NULLREQUEST_HH
#define IOFWD_NULLREQUEST_HH

#include "Request.hh"

namespace iofwd
{

class NullRequest : public Request
{
public:
   NullRequest (int opid) 
      : Request (opid)
   {
   }

   virtual ~NullRequest (); 
}; 

}

#endif
