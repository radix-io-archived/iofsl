#ifndef IOFWDUTIL_WORKQUEUE_WORKITEM_HH
#define IOFWDUTIL_WORKQUEUE_WORKITEM_HH

#include "CompletionTracker.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================


class SynchronousWorkQueue; 
class WorkQueueBase; 

class WorkItem 
{
public:

   WorkItem () 
      : workitem_completed_ (false)
   {
   }

   virtual void doWork () = 0; 

   virtual ~WorkItem (); 

protected:
   friend class WorkQueueBase;

   CompletionTracker::ID getID () const
   { return id_; }

   void setID (CompletionTracker::ID id) 
   { id_ = id; }

private:
   friend class CompletionTracker;

   void setCompleted ()
   {
      boost::mutex::scoped_lock l (lock_);
      workitem_completed_ = true;
      ready_.notify_all();
   }

   bool hasCompleted () const
   {
      boost::mutex::scoped_lock l (lock_);
      return workitem_completed_;
   }

   void waitCompleted ()
   {
      boost::mutex::scoped_lock l (lock_);
      while (!workitem_completed_)
         ready_.wait(l);
   }

private:
   
   bool workitem_completed_;
   boost::condition_variable ready_;
   mutable boost::mutex lock_;

   CompletionTracker::ID id_; 

}; 

//===========================================================================
   }
}


#endif
