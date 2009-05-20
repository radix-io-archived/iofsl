#include <boost/bind.hpp>
#include <functional>
#include "DefRequestItem.hh"
#include "iofwdutil/workqueue/WorkQueue.hh"
#include "Request.hh"
#include "iofwdutil/assert.hh"

using namespace iofwdutil::workqueue; 

namespace iofwd
{
//===========================================================================

   DefRequestItem::DefRequestItem (WorkQueue * queue, 
         Request * request)
      : queue_ (queue), request_(request)
   {
      /*request_->setResume (boost::bind (&WorkQueue::queueWork, queue_,
               this));  */
 /*     request_->setReschedule
         (std::bind1st(std::mem_fun(&DefReqyestItem::reschedule), this)); */
   }


   void DefRequestItem::doWork ()
   {
      /*switch (request_->run ())
      {
      case Request::STATUS_DONE:
         // Request is completely done. Have the workqueue free it
         break;
      case Request::STATUS_RERUN:
         // Request can rerun immediately. Reschedule
         reschedule (); 
         break;
      case Request::STATUS_WAITING:
         // Some external entity is going to reschedule us.
         break; 
      default:
         ALWAYS_ASSERT(false && "Invalid return code from Request::run()"); 
      }*/
   }

   DefRequestItem::~DefRequestItem ()
   {
   }

   void  DefRequestItem::reschedule ()
   {
      queue_->queueWork (this); 
   }

//===========================================================================
}

