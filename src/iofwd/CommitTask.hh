#ifndef IOFWD_COMMITTASK_HH
#define IOFWD_COMMITTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "CommitRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class CommitTask : public Task, public TaskHelper<CommitRequest>
{
public:
   CommitTask (Request * req, boost::function<void (Task*)>
         & resched, zoidfs::ZoidFSAPI * api) 
      : Task (resched), TaskHelper<CommitRequest>(req, api)
   {
   }

   void run ()
   {
      //request_.setReturnCode (api_->commit (handle)); 
      request_.reply (); 
   }

   virtual ~CommitTask (); 
}; 

}


#endif
