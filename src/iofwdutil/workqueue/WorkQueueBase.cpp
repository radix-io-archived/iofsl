#include "WorkQueueBase.hh"
#include "WorkItem.hh"
#include "iofwdutil/assert.hh"

namespace iofwdutil
{
   namespace workqueue
   {
//===========================================================================

void WorkQueueBase::waitAll (std::vector<WorkItem *> & items)
{
   tracker_.waitAll (std::back_inserter(items)); 
}

void WorkQueueBase::testAll (std::vector<WorkItem *> & items)
{
   tracker_.testAll (std::back_inserter(items)); 
}

void WorkQueueBase::completeItem (WorkItem * item)
{
   // no need for locking: tracker locks if needed
   CompletionTracker::ID id = item->getID (); 
   tracker_.completed (id); 
}

bool WorkQueueBase::isBusy () const
{
   return tracker_.isBusy (); 
}

void WorkQueueBase::newItem (WorkItem* item )
{
   // no need for locking
   item->setID (tracker_.createID (item)); 
}

WorkQueueBase::~WorkQueueBase ()
{
   ALWAYS_ASSERT (!isBusy()); 
}
//===========================================================================


   }
}
