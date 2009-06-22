#include "SynchronousWorkQueue.hh"
#include "WorkItem.hh"
#include "CompletionTracker.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/completion/WorkQueueCompletionID.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

SynchronousWorkQueue::~SynchronousWorkQueue ()
{
}

iofwdutil::completion::CompletionID * SynchronousWorkQueue::queueWork (WorkItem * item)
{
   newItem (item); 

   iofwdutil::completion::WorkQueueCompletionID * id = new iofwdutil::completion::WorkQueueCompletionID ();
   id->tracker_ = &tracker_;
   id->tracker_id_ = item;

   try
   {
      item->doWork (); 
      completeItem (item); 
   }
   catch (...)
   {
      ASSERT (false && "Exception during work execution!"); 
   }
   return id;
}

//===========================================================================
     }
}
