#ifndef IOFWD_NULLTASK_HH
#define IOFWD_NULLTASK_HH

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <memory>
#include "Task.hh"
#include "NullRequest.hh"
#include "TaskHelper.hh"
#include "zoidfs/util/ZoidFSAPI.hh"
#include "iofwdevent/SingleCompletion.hh"

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
      // Call the ZOIDFS_NULL function and store the return code.
      request_.setReturnCode (api_->null ());

      request_.reply (boost::ref(block_));
      block_.wait ();
   }

};

}


#endif
