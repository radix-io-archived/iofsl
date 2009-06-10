#ifndef IOFWD_NULLTASK_HH
#define IOFWD_NULLTASK_HH

#include <boost/function.hpp>
#include <memory>
#include "Task.hh"
#include "NullRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"

namespace iofwd
{

class NullTask : public TaskHelper<NullRequest>
{
public:
   NullTask (ThreadTaskParam & p)
      : TaskHelper<NullRequest>(p)
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
      std::auto_ptr<iofwdutil::completion::CompletionID>  id (request_.reply ()); 
      id->wait (); 
   }

   virtual ~NullTask (); 

}; 

}


#endif
