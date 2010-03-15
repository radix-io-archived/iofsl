#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/construct.hpp>
#include "DefRequestHandler.hh"
#include "Request.hh"
#include "Task.hh"
#include "iofwdutil/workqueue/SynchronousWorkQueue.hh"
#include "iofwdutil/workqueue/PoolWorkQueue.hh"
#include "ThreadTasks.hh"
#include "iofwdutil/workqueue/WorkItem.hh"
#include "zoidfs/zoidfs-proto.h"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"
#include "RequestScheduler.hh"

#include "iofwd/TaskPoolHelper.hh"

using namespace iofwdutil;
using namespace iofwdutil::workqueue;
using namespace boost::lambda;

namespace iofwd
{
//===========================================================================

DefRequestHandler::DefRequestHandler (const iofwdutil::ConfigFile & c)
   : log_ (IOFWDLog::getSource ("defreqhandler")), config_(c)
{
   iofwdutil::ConfigFile csec;
   if (api_.init(config_.openSectionDefault ("zoidfsapi")) != zoidfs::ZFS_OK)
      throw "ZoidFSAPI::init() failed";

   csec = config_.openSectionDefault("zoidfsasyncapi");
   async_api_ = new zoidfs::ZoidFSAsyncAPI(&api_,
                                           csec.getKeyAsDefault("minthreadnum", 0),
                                           csec.getKeyAsDefault("maxthreadnum", 100));
   sched_ = new RequestScheduler(async_api_, config_.openSectionDefault ("requestscheduler"));
   bpool_ = new BMIBufferPool(config_.openSectionDefault("bmibufferpool"));

   csec = config_.openSectionDefault("workqueue");
   workqueue_normal_.reset (new PoolWorkQueue (csec.getKeyAsDefault("minthreadnum", 0),
                                               csec.getKeyAsDefault("maxthreadnum", 100)));
   workqueue_fast_.reset (new SynchronousWorkQueue ());
   boost::function<void (Task *)> f = boost::lambda::bind
      (&DefRequestHandler::reschedule, this, boost::lambda::_1);

   tpool_ = new TaskPool(config_.openSectionDefault("taskpool"), f);
   taskfactory_.reset (new ThreadTasks (f, &api_, async_api_, sched_, bpool_, tpool_));
}

void DefRequestHandler::reschedule (Task * t)
{
   workqueue_normal_->queueWork (t);
}

DefRequestHandler::~DefRequestHandler ()
{
   std::vector<WorkItem *> items;
   ZLOG_INFO (log_, "Waiting for normal workqueue to complete all work...");
   workqueue_normal_->waitAll (items);
   for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1));

   items.clear();
   ZLOG_INFO (log_, "Waiting for fast workqueue to complete all work...");
   workqueue_fast_->waitAll (items);
   for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1));

   delete sched_;
   delete bpool_;
   delete tpool_;

   api_.finalize();
   delete async_api_;
}

void DefRequestHandler::handleRequest (int count, Request ** reqs)
{
   ZLOG_DEBUG(log_, str(format("handleRequest: %u requests") % count));
   for (int i=0; i<count; ++i)
   {
      Task * task = (*taskfactory_) (reqs[i]);

      // TODO: workqueues are supposed to return some handle so that we can
      // test which requests completed. That way that requesthandler can
      // reschedule requests and free completed requests
      iofwdutil::completion::CompletionID * id;
      if (task->isFast())
         id = workqueue_fast_->queueWork (task);
      else
         id = workqueue_normal_->queueWork (task);
      delete id;
   }

   // Cleanup completed requests
   workqueue_normal_->testAll (completed_);
   workqueue_fast_->testAll (completed_);
   for (unsigned int i=0; i<completed_.size(); ++i)
   {
      // we know only requesttasks can be put on the workqueues
      if (static_cast<Task*>(completed_[i])->getStatus() ==
            Task::STATUS_DONE)
      {
#ifndef USE_TASK_POOL_ALLOCATOR
#ifdef USE_IOFWD_TASK_POOL 
         /* if this task was allocated from the pool, put it back on the pool */
         if(static_cast<Task*>(completed_[i])->getTaskAllocType() == true)
         {
            static_cast<Task*>(completed_[i])->cleanup();
         }
         /* else, delete it */
         else
         {
            delete (completed_[i]);
         }
#else
            delete (completed_[i]);
#endif
#else
        /* invoke the destructor and then add the mem back the task memory pool */
        static_cast<Task *>(completed_[i])->~Task();
        iofwd::TaskPoolAllocator::instance().deallocate(static_cast<Task *>(completed_[i]));
#endif
      }
   }
   completed_.clear ();
}

//===========================================================================
}
