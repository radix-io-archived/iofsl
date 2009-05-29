#ifndef IOFWD_NOTIMPLEMENTEDTASK_HH
#define IOFWD_NOTIMPLEMENTEDTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "NotImplementedRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{
//===========================================================================


class NotImplementedTask : public Task, 
   public TaskHelper<NotImplementedRequest> 
{
public:
   NotImplementedTask (Request * req, boost::function<void (Task*)>
         & resched, zoidfs::ZoidFSAPI * api) 
      : Task (resched), TaskHelper<NotImplementedRequest>(req, api)
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
