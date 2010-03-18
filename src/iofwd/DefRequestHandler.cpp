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
#include "zoidfs/util/ZoidFSDefAsync.hh"
#include "RequestScheduler.hh"
#include "iofwd/tasksm/WriteTaskSM.hh"

#include "iofwd/TaskPoolHelper.hh"

using namespace iofwdutil;
using namespace iofwdutil::workqueue;
using namespace boost::lambda;

namespace iofwd
{
//===========================================================================

DefRequestHandler::DefRequestHandler (const iofwdutil::ConfigFile & c)
   : log_ (IOFWDLog::getSource ("defreqhandler")), config_(c), event_mode_(EVMODE_SM)
{

   if (api_.init(config_.openSectionDefault ("zoidfsapi")) != zoidfs::ZFS_OK)
      throw "ZoidFSAPI::init() failed";

   /* start thread pool */
   iofwdutil::ConfigFile lc = config_.openSectionDefault ("threadpool");
   iofwdutil::ThreadPool::instance().setMinThreadCount(lc.getKeyAsDefault("minnumthreads", 0));
   iofwdutil::ThreadPool::instance().setMaxThreadCount(lc.getKeyAsDefault("maxnumthreads", 0));
   iofwdutil::ThreadPool::instance().start();

   /* get the event mode */
   lc = config_.openSectionDefault ("events");
   char * evmode = new char[lc.getKeyDefault("mode", "SM").size() + 1];
   strcpy(evmode, const_cast<char *>(lc.getKeyDefault ("mode", "SM").c_str()));
   if(strcmp(evmode, "SM") == 0)
   {
        event_mode_ = EVMODE_SM;
   }
   else if(strcmp(evmode, "TASK") == 0)
   {
        event_mode_ = EVMODE_TASK;
   }
   delete [] evmode;

   async_api_ = new zoidfs::ZoidFSAsyncAPI(&api_);
   async_api_full_ = new zoidfs::util::ZoidFSDefAsync(api_);
   sched_ = new RequestScheduler(async_api_, async_api_full_, config_.openSectionDefault ("requestscheduler"), event_mode_);
   bpool_ = new BMIBufferPool(config_.openSectionDefault("bmibufferpool"));

   workqueue_normal_.reset (new PoolWorkQueue (0, 100));
   workqueue_fast_.reset (new SynchronousWorkQueue ());
   boost::function<void (Task *)> f = boost::lambda::bind
      (&DefRequestHandler::reschedule, this, boost::lambda::_1);

   taskfactory_.reset (new ThreadTasks (f, &api_, async_api_, sched_, bpool_));

   taskSMFactory_.reset(new iofwd::tasksm::TaskSMFactory(sched_, bpool_, smm, async_api_full_));
   smm.startThreads();
}

void DefRequestHandler::reschedule (Task * t)
{
   workqueue_normal_->queueWork (t);
}

DefRequestHandler::~DefRequestHandler ()
{
   /* if this is the state machine mode, shutdown the state machine manager */
   if(event_mode_ == EVMODE_SM)
   {
        smm.stopThreads();
   }
   /* if this is the task mode, clear out the work queues before shutdown */
   else if(event_mode_ == EVMODE_TASK)
   {
        std::vector<WorkItem *> items;
        ZLOG_INFO (log_, "Waiting for normal workqueue to complete all work...");
        workqueue_normal_->waitAll (items);
        for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1));

        items.clear();
        ZLOG_INFO (log_, "Waiting for fast workqueue to complete all work...");
        workqueue_fast_->waitAll (items);
        for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1));
   }

   delete sched_;
   delete bpool_;

   api_.finalize();
   delete async_api_;
   delete async_api_full_;
}

void DefRequestHandler::handleRequest (int count, Request ** reqs)
{
    ZLOG_DEBUG(log_, str(format("handleRequest: %u requests") % count));
    for (int i=0; i<count; ++i)
    {
        if(event_mode_ == EVMODE_SM)
        {
            (*taskSMFactory_)(reqs[i]);
        }
        else if(event_mode_ == EVMODE_TASK)
        {
            Task * task = (*taskfactory_) (reqs[i]);
            iofwdutil::completion::CompletionID * id;
            if (task->isFast())
                id = workqueue_fast_->queueWork (task);
            else
                id = workqueue_normal_->queueWork (task);
            delete id;
        }
    }

    if(event_mode_ == EVMODE_TASK)
    {
        // Cleanup completed requests
        workqueue_normal_->testAll (completed_);
        workqueue_fast_->testAll (completed_);
        for (unsigned int i=0; i<completed_.size(); ++i)
        {
            // we know only requesttasks can be put on the workqueues
            if (static_cast<Task*>(completed_[i])->getStatus() ==
                Task::STATUS_DONE)
            {
                /* invoke the destructor and then add the mem back the task memory pool */
                static_cast<Task *>(completed_[i])->~Task();
                iofwd::TaskPoolAllocator::instance().deallocate(static_cast<Task *>(completed_[i]));
            }
        }
        completed_.clear ();
    }
}

//===========================================================================
}
