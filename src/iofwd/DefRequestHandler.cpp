#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include "DefRequestHandler.hh"
#include "Request.hh"
#include "iofwdutil/workqueue/SynchronousWorkQueue.hh"
#include "DefRequestItem.hh"

using namespace iofwdutil; 
using namespace iofwdutil::workqueue;
using namespace boost::lambda; 

namespace iofwd
{
//===========================================================================

DefRequestHandler::DefRequestHandler ()
   : log_ (IOFWDLog::getSource ("defreqhandler"))
{
   workqueue_normal_.reset (new SynchronousWorkQueue ()); 
   workqueue_fast_.reset (new SynchronousWorkQueue ()); 
}

DefRequestHandler::~DefRequestHandler ()
{
   std::vector<WorkItem *> items; 
   ZLOG_INFO (log_, "Waiting for normal workqueue to complete all work..."); 
   workqueue_normal_->waitAll (items); 
   for_each (items.begin(), items.end(), bind(delete_ptr(), _1)); 

   ZLOG_INFO (log_, "Waiting for fast workqueue to complete all work..."); 
   workqueue_fast_->waitAll (items); 
   for_each (items.begin(), items.end(), bind(delete_ptr(), _1)); 
}

void DefRequestHandler::handleRequest (int count, Request ** reqs)
{
   ZLOG_DEBUG(log_, str(format("handleRequest: %u requests") % count)); 
   for (int i=0; i<count; ++i)
   {
      // TODO: workqueues are supposed to return some handle so that we can
      // test which requests completed. That way that requesthandler can
      // reschedule requests and free completed requests
      if (reqs[i]->isFast())
         workqueue_fast_->queueWork (new DefRequestItem (workqueue_fast_.get(), 
                  reqs[i]));
      else
         workqueue_normal_->queueWork (new DefRequestItem
               (workqueue_normal_.get(), reqs[i])); 
   }

   // Cleanup completed requests
   workqueue_normal_->testAll (completed_); 
   workqueue_fast_->testAll (completed_); 
   std::for_each(completed_.begin(), completed_.end(), bind(delete_ptr(),
            _1)); 
   completed_.clear (); 
}

//===========================================================================
}
