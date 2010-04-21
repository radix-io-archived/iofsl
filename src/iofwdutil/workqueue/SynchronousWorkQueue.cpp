#include "SynchronousWorkQueue.hh"
#include "WorkItem.hh"
#include "iofwdutil/assert.hh"

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
   newItem (item); 

   try
   {
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
