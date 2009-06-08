#ifndef IOFWD_COMMITTASK_HH
#define IOFWD_COMMITTASK_HH

#include <boost/function.hpp>
#include "Task.hh"
#include "CommitRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class CommitTask : public TaskHelper<CommitRequest>
{
public:
   CommitTask (ThreadTaskParam & p)
      : TaskHelper<CommitRequest>(p)
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
