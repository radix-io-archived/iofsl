#ifndef IOFWDUTIL_WORKQUEUE_SYNCHRONOUSWORKQUEUE_HH
#define IOFWDUTIL_WORKQUEUE_SYNCHRONOUSWORKQUEUE_HH

#include "WorkQueueBase.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

/**
 * Executes workitem in the context of the calling thread
 * when they are queued.
 * Does not use other worker threads.
 */
class SynchronousWorkQueue : public WorkQueueBase
{
public:

   virtual void queueWork (WorkItem * work); 

   virtual ~SynchronousWorkQueue (); 
};

//===========================================================================
   }
}

#endif
