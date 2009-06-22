#ifndef IOFWDUTIL_COMPLETION_WORKQUEUECOMPLETIONID_HH
#define IOFWDUTIL_COMPLETION_WORKQUEUECOMPLETIONID_HH

#include "CompletionID.hh"
#include "iofwdutil/workqueue/CompletionTracker.hh"

namespace iofwdutil
{
   namespace workqueue
   {
      class WorkQueue;
      class SynchronousWorkQueue;
      class PoolWorkQueue;
   }

   namespace completion
   {
//===========================================================================

class WorkQueueCompletionID : public CompletionID 
{
public:

   WorkQueueCompletionID ()
      : tracker_(NULL)
   {
   }
   
   virtual void wait ();

   virtual bool test (unsigned int maxms);

protected:
   friend class iofwdutil::workqueue::SynchronousWorkQueue;
   friend class iofwdutil::workqueue::PoolWorkQueue;

   iofwdutil::workqueue::CompletionTracker * tracker_;
   iofwdutil::workqueue::CompletionTracker::value_type tracker_id_;
};

//===========================================================================
   }
}

#endif
