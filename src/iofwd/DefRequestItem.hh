#ifndef IOFWD_DEFREQUESTITEM_HH
#define IOFWD_DEFREQUESTITEM_HH

#include "iofwdutil/workqueue/WorkItem.hh"

namespace iofwdutil
{
   namespace workqueue
   {
      class WorkQueue;
   }
}

namespace iofwd
{
//===========================================================================

/// Forward
class Request; 

/**
 * Packages requests so that they can be put on workqueues.
 * Takes care of rescheduling the request to the right workqueue
 * or putting it on hold if temporary blocked.
 *
 */
class DefRequestItem : public iofwdutil::workqueue::WorkItem
{
public:
   DefRequestItem (iofwdutil::workqueue::WorkQueue * queue, Request * req);

   virtual void doWork (); 

   virtual ~DefRequestItem (); 

protected:
   /// Called to reschedule the item on the work queue
   void reschedule (); 

protected:
   iofwdutil::workqueue::WorkQueue * queue_; 
   Request *                         request_; 
}; 

//===========================================================================
}
#endif
