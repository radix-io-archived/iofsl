#ifndef IOFWD_NOTIMPLEMENTEDTASK_HH
#define IOFWD_NOTIMPLEMENTEDTASK_HH

#include <boost/function.hpp>
#include "RequestTask.hh"
#include "NotImplementedRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{
//===========================================================================


class NotImplementedTask : public RequestTask, 
   public TaskHelper<NotImplementedRequest> 
{
public:
   NotImplementedTask (Request * req, boost::function<void (RequestTask*)>
         & resched) 
      : RequestTask (resched), TaskHelper<NotImplementedRequest>(req)
   {
   }

   /// Not implemented is a fast request. No need to schedule it 
   bool isFast () const; 

   void run ()
   {
      request_.reply (); 
   }
}; 

//===========================================================================
}

#endif
