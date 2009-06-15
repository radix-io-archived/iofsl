#ifndef IOFWD_WRITEREQUEST_HH
#define IOFWD_WRITEREQUEST_HH

#include "Request.hh"

namespace iofwd
{
   
class WriteRequest : public Request 
{
public:

  WriteRequest (int opid)
     : Request (opid)
  {
  }
  virtual ~WriteRequest ()
  {
  }
}; 

}

#endif
