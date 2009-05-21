#ifndef IOFWD_NULLTASK_HH
#define IOFWD_NULLTASK_HH

#include <boost/function.hpp>
#include "RequestTask.hh"
#include "NullRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

class NullTask : public RequestTask, public TaskHelper<NullRequest>
{
public:
   NullTask (Request * req, boost::function<void (RequestTask*)>
         & resched) 
      : RequestTask (resched), TaskHelper<NullRequest>(req)
   {
   }

   /// Not implemented is a fast request. No need to schedule it 
   bool isFast () const
   {
      return true; 
   }

   void run ()
   {
      request_.reply (); 
   }

}; 

}


#endif
