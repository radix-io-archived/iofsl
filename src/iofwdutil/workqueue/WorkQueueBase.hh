#ifndef IOFWDUTIL_WORKQUEUE_WORKQUEUEBASE_HH
#define IOFWDUTIL_WORKQUEUE_WORKQUEUEBASE_HH

#include "WorkQueue.hh"
#include "CompletionTracker.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

 /**
  * Provides basic completion tracking support
  */
class WorkQueueBase : public WorkQueue
{
   protected:
      

      /// called when a request is ready
      void completeItem (WorkItem * item); 

      /// Called when a workitem is received
      void newItem (WorkItem * item); 

      
   public:
      virtual void waitAll (std::vector<WorkItem *> & items); 

      virtual void testAll (std::vector<WorkItem *> & items); 

      virtual bool isBusy () const; 

      virtual ~WorkQueueBase (); 

   protected:
      CompletionTracker tracker_; 

};

//===========================================================================
   }
}

#endif

