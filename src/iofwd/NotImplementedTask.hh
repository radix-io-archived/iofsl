#ifndef IOFWD_NOTIMPLEMENTEDTASK_HH
#define IOFWD_NOTIMPLEMENTEDTASK_HH

#include <boost/function.hpp>
#include "RequestTask.hh"
#include "NotImplementedRequest.hh"

namespace iofwd
{
//===========================================================================


class NotImplementedTask : public RequestTask
{
public:
   NotImplementedTask (Request * req, boost::function<void (RequestTask*)>
         & resched) 
      : RequestTask (resched)
   {
      setRequest (req); 
      //request_ = dynamic_cast<NotImplementedRequest*> (req); 
   }

   /// Not implemented is a fast request. No need to schedule it 
   bool isFast () const; 

   void run ()
   {
   }
protected:
   NotImplementedRequest * request_; 
}; 

//===========================================================================
}

#endif
