#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include "DefRequestHandler.hh"
#include "Request.hh"
#include "RequestTask.hh"
#include "iofwdutil/workqueue/SynchronousWorkQueue.hh"
#include "iofwdutil/workqueue/PoolWorkQueue.hh"
#include "ThreadTasks.hh"
#include "iofwdutil/workqueue/WorkItem.hh"

using namespace iofwdutil; 
using namespace iofwdutil::workqueue;
using namespace boost::lambda; 

namespace iofwd
{
//===========================================================================

DefRequestHandler::DefRequestHandler ()
   : log_ (IOFWDLog::getSource ("defreqhandler"))
{
   workqueue_normal_.reset (new PoolWorkQueue (0, 100)); 
   workqueue_fast_.reset (new SynchronousWorkQueue ()); 
   boost::function<void (RequestTask *)> f = boost::lambda::bind
      (&DefRequestHandler::reschedule, this, boost::lambda::_1); 
   taskfactory_.reset (new ThreadTasks (f)); 
}

void DefRequestHandler::reschedule (RequestTask * t)
{
   workqueue_normal_->queueWork (t); 
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
      RequestTask * task = (*taskfactory_) (reqs[i]); 

      // TODO: workqueues are supposed to return some handle so that we can
      // test which requests completed. That way that requesthandler can
      // reschedule requests and free completed requests
      if (task->isFast())
         workqueue_fast_->queueWork (task);
      else
         workqueue_normal_->queueWork (task); 
   }

   // Cleanup completed requests
   workqueue_normal_->testAll (completed_); 
   workqueue_fast_->testAll (completed_); 
   for (unsigned int i=0; i<completed_.size(); ++i)
   {
      // we know only requesttasks can be put on the workqueues
      if (static_cast<RequestTask*>(completed_[i])->getStatus() ==
            RequestTask::STATUS_DONE)
         delete (completed_[i]); 
   }
   completed_.clear (); 
}

//===========================================================================
}
