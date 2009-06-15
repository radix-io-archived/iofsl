#ifndef IOFWD_NOTIMPLEMENTEDREQUEST_HH
#define IOFWD_NOTIMPLEMENTEDREQUEST_HH

#include "Request.hh"

namespace iofwd
{
//===========================================================================

class NotImplementedRequest : public Request
{
public:
   NotImplementedRequest (int opid)
      : Request (opid)
   {
   }
   virtual ~NotImplementedRequest ()
   {
   }

   virtual void reply () = 0; 

};


//===========================================================================
}

#endif
