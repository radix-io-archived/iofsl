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
#include "iofwdutil/Factory.hh"
#include "iofwdutil/FactoryException.hh"

using namespace iofwdutil;
using namespace iofwdutil::workqueue;
using namespace boost::lambda;

namespace iofwd
{
//===========================================================================

DefRequestHandler::DefRequestHandler (const iofwdutil::ConfigFile & cf)
   : log_ (IOFWDLog::getSource ("defreqhandler")), config_(cf), event_mode_(EVMODE_SM)
{
   iofwdutil::ConfigFile csec;
   if (api_.init(config_.openSectionDefault ("zoidfsapi")) != zoidfs::ZFS_OK)
      throw "ZoidFSAPI::init() failed";

   /* start thread pool */
   iofwdutil::ConfigFile lc = config_.openSectionDefault ("threadpool");
   iofwdutil::ThreadPool::instance().setMinThreadCount(lc.getKeyAsDefault("minnumthreads", 0));
   iofwdutil::ThreadPool::instance().setMaxThreadCount(lc.getKeyAsDefault("maxnumthreads", 0));
   iofwdutil::ThreadPool::instance().start();

   /* get the event mode */
   lc = config_.openSectionDefault ("events");
   const std::string mode = lc.getKeyDefault("mode", "SM");
   if(mode == "SM")
   {
      ZLOG_INFO(log_, "Using SM mode");
      event_mode_ = EVMODE_SM;
   }
   else if (mode == "TASK")
   {
      ZLOG_INFO(log_, "Using TASK mode");
      event_mode_ = EVMODE_TASK;
   }
   else
   {
      ZLOG_WARN(log_, format("Invalid mode '%s' in [events]; Defaulting to SM")
            % mode);
      event_mode_ = EVMODE_SM;
   }

   async_api_ = new zoidfs::ZoidFSAsyncAPI(&api_);
 
   const std::string apiname = config_.getKeyAsDefault<std::string>("zoidfsapi.name","defasync");
   try
   {
      ZLOG_INFO (log_, format("Loading using async API '%s'") % apiname);
      async_api_full_ = iofwdutil::Factory<
                        std::string,
                        zoidfs::util::ZoidFSAsync>::construct (apiname)(api_);
   }
   catch (FactoryException & e)
   {
      ZLOG_ERROR(log_, format("No async API called '%s' registered!") %
            apiname);
      throw;
   }

   sched_ = new RequestScheduler(async_api_, async_api_full_, config_.openSectionDefault ("requestscheduler"), event_mode_);

   /* start the BMI memory manager */
   lc = config_.openSectionDefault("bmimemorymanager");
   iofwd::BMIMemoryManager::instance().setMaxBufferSize(lc.getKeyAsDefault("buffersize", 0));
   iofwd::BMIMemoryManager::instance().setMaxNumBuffers(lc.getKeyAsDefault("maxnumbuffers", 0));
   iofwd::BMIMemoryManager::instance().start();

   csec = config_.openSectionDefault("workqueue");
   workqueue_normal_.reset (new PoolWorkQueue (csec.getKeyAsDefault("minthreadnum", 0),
                                               csec.getKeyAsDefault("maxthreadnum", 100)));
   workqueue_fast_.reset (new SynchronousWorkQueue ());

   taskfactory_.reset (new ThreadTasks (&api_, async_api_, sched_));

   taskSMFactory_.reset(new iofwd::tasksm::TaskSMFactory(sched_, smm,
            async_api_full_));
   smm.startThreads();
}

DefRequestHandler::~DefRequestHandler ()
{
   /* if this is the state machine mode, shutdown the state machine manager */
   if(event_mode_ == EVMODE_SM)
   {
      // @TODO we need to wait for state machines too!
        smm.stopThreads();
   }
   /* if this is the task mode, clear out the work queues before shutdown */
   else if(event_mode_ == EVMODE_TASK)
   {
        smm.stopThreads();

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
            if (task->isFast())
                workqueue_fast_->queueWork (task);
            else
                workqueue_normal_->queueWork (task);
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
           delete (completed_[i]);
        }
        completed_.clear ();
    }
}

//===========================================================================
}
