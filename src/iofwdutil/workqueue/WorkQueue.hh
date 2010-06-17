#ifndef IOFWDUTIL_WORKQUEUE_WORKQUEUE_HH
#define IOFWDUTIL_WORKQUEUE_WORKQUEUE_HH

#include <vector>

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

// Forward
class WorkItem; 

/**
 * Queue en dequeue work packets.
 * Allows polling for completion 
 *
 */
class WorkQueue
{
public:

   virtual void queueWork (WorkItem * work) = 0; 

   /// Returns true if work is still in progress
   virtual bool isBusy () const = 0;  


   virtual void waitAll (std::vector<WorkItem*> &out ) = 0; 

   /// Return list of completed workitems 
   virtual void testAll (std::vector<WorkItem *> & items) = 0; 

   virtual ~WorkQueue (); 

}; 

//===========================================================================
   }
}

#endif
