#include "CompletionTracker.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/workqueue/WorkItem.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//=======================================================================

CompletionTracker::CompletionTracker ()
   : pending_(0)
{
}

CompletionTracker::ID CompletionTracker::createID (value_type val)
{
   boost::mutex::scoped_lock  l(lock_); 

   active_.insert (val); 

   ++pending_; 
   return val; 
}


bool CompletionTracker::test (ID id)
{
   return id->hasCompleted();
}

void CompletionTracker::wait (ID id)
{
   id->waitCompleted ();
}

bool CompletionTracker::isBusy () const
{
   boost::mutex::scoped_lock l (lock_); 
   return !completed_.empty() || !active_.empty(); 
}

void CompletionTracker::completed (ID id)
{
   boost::mutex::scoped_lock  l(lock_); 

   ALWAYS_ASSERT(pending_); 
   --pending_; 

   ASSERT(active_.count (id));
   completed_.push_back (id); 
   id->setCompleted ();
   active_.erase (id); 

   // notify threads waiting on completion
   ready_.notify_one();
}

//=======================================================================
   }
}
