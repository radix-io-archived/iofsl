#ifndef IOFWD_NULLTASK_HH
#define IOFWD_NULLTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "NullRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class NullTask : public Task, public TaskHelper<NullRequest>
{
public:
   NullTask (Request * req, boost::function<void (Task*)>
         & resched, zoidfs::ZoidFSAPI * api) 
      : Task (resched), TaskHelper<NullRequest>(req, api)
   {
   }

   /// zoidfs_null is a fast request. No need to schedule it 
   bool isFast () const
   {
      return true; 
   }

   void run ()
   {
      request_.setReturnCode (api_->null ()); 
      request_.reply (); 
   }

   virtual ~NullTask (); 

}; 

}


#endif
