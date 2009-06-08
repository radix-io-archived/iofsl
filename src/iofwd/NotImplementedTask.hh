#ifndef IOFWD_NOTIMPLEMENTEDTASK_HH
#define IOFWD_NOTIMPLEMENTEDTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "NotImplementedRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{
//===========================================================================


class NotImplementedTask : 
   public TaskHelper<NotImplementedRequest> 
{
public:
   NotImplementedTask (ThreadTaskParam & p)
      : TaskHelper<NotImplementedRequest>(p)
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
