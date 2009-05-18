#include "SynchronousWorkQueue.hh"
#include "WorkItem.hh"
#include "iofwdutil/assert.hh"
#include "CompletionTracker.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

SynchronousWorkQueue::~SynchronousWorkQueue ()
{
}

void SynchronousWorkQueue::queueWork (WorkItem * item)
{
   try
   {
      newItem (item); 
      item->doWork (); 
      completeItem (item); 
   }
   catch (...)
   {
      ASSERT (false && "Exception during work execution!"); 
   }
}

//===========================================================================
     }
}
