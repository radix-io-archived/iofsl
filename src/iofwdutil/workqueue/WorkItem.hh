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

   bool hasCompleted () const
   { return workitem_completed_; }

private:
   
   bool workitem_completed_; 

   CompletionTracker::ID id_; 

}; 

//===========================================================================
   }
}


#endif
