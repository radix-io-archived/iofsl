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
#include "iofwdutil/Factory.hh"
#include "iofwdutil/FactoryException.hh"
#include "iofwdutil/ZException.hh"

using namespace iofwdutil;
using namespace iofwdutil::workqueue;
using namespace boost::lambda;

namespace iofwd
{
//===========================================================================

DefRequestHandler::DefRequestHandler (const iofwdutil::ConfigFile & cf)
   : log_ (IOFWDLog::getSource ("defreqhandler")), config_(cf), event_mode_(EVMODE_SM), tp_(iofwdutil::ThreadPool::instance())
{
   //
   // Get async API
   //
   const iofwdutil::ConfigFile zoidfssection (config_.openSectionDefault ("zoidfsapi"));
   const std::string apiname = zoidfssection.getKeyAsDefault<std::string>("name","defasync");
   try
   {
      ZLOG_INFO (log_, format("Loading using async API '%s'") % apiname);
      api_.reset (iofwdutil::Factory<
                        std::string,
                        zoidfs::util::ZoidFSAsync>::construct (apiname)());
   }
   catch (FactoryException & e)
   {
      ZLOG_ERROR(log_, format("No async API called '%s' registered!") %
            apiname);
      throw;
   }

   // We configure the API using its own subsection 
   iofwdutil::Configurable::configure_if_needed (api_.get(),
         zoidfssection.openSectionDefault(apiname.c_str()));

   if (api_->init() != zoidfs::ZFS_OK)
   {
      ZTHROW(iofwdutil::ZException ()
            << zexception_msg("ZoidFSAPI::init() failed"));
   }

   /* start thread pool */
   iofwdutil::ConfigFile lc = config_.openSectionDefault ("threadpool");

   /* parse the norm prio thread section */
   iofwdutil::ConfigFile ntc = lc.openSectionDefault ("normprio");
   std::vector<std::string> norm_max_thread_params;
   std::vector<std::string> norm_warn_thread_params;
   std::string norm_max_limits = ntc.getKeyAsDefault<std::string>("maxlimits",
           "FILEIO:8,SM:4,RPC:4,OTHER:0");
   std::string norm_warn_limits = ntc.getKeyAsDefault<std::string>("warnlimits",
           "FILEIO:1,SM:1,RPC:1,OTHER:0");

   boost::split(norm_max_thread_params, norm_max_limits, boost::is_any_of(",:"));
   boost::split(norm_warn_thread_params, norm_warn_limits, boost::is_any_of(",:"));

   for(unsigned int i = 0 ; i < norm_max_thread_params.size() ; i+=2)
   {
        tp_.setMaxNormThreadCount(
                iofwdutil::ThreadPool::getAttrType(norm_max_thread_params[i]),
                boost::lexical_cast<int>(norm_max_thread_params[i + 1]));
   }

   for(unsigned int i = 0 ; i < norm_warn_thread_params.size() ; i+=2)
   {
        tp_.setNormThreadWarn(
                iofwdutil::ThreadPool::getAttrType(norm_warn_thread_params[i]),
                boost::lexical_cast<int>(norm_warn_thread_params[i + 1]));
   }

   /* parse the high prio thread section */
   iofwdutil::ConfigFile htc = lc.openSectionDefault ("highprio");
   std::vector<std::string> high_max_thread_params;
   std::vector<std::string> high_warn_thread_params;
   std::string high_max_limits = htc.getKeyAsDefault<std::string>("maxlimits",
           "FILEIO:8,SM:4,RPC:4,OTHER:0");
   std::string high_warn_limits = htc.getKeyAsDefault<std::string>("warnlimits",
           "FILEIO:1,SM:1,RPC:1,OTHER:0");

   boost::split(high_max_thread_params, high_max_limits, boost::is_any_of(",:"));
   boost::split(high_warn_thread_params, high_warn_limits, boost::is_any_of(",:"));

   for(unsigned int i = 0 ; i < high_max_thread_params.size() ; i+=2)
   {
        tp_.setMaxHighThreadCount(
                iofwdutil::ThreadPool::getAttrType(high_max_thread_params[i]),
                boost::lexical_cast<int>(high_max_thread_params[i + 1]));
   }

   for(unsigned int i = 0 ; i < high_warn_thread_params.size() ; i+=2)
   {
        tp_.setHighThreadWarn(
                iofwdutil::ThreadPool::getAttrType(high_warn_thread_params[i]),
                boost::lexical_cast<int>(high_warn_thread_params[i + 1]));
   }

   /* start the thread pool */
   tp_.start();

   /* get the event mode */
   lc = config_.openSectionDefault ("events");
   const std::string mode = lc.getKeyDefault("mode", "SM");
   if(mode == "SM")
   {
      bool smhighprio = true;
      ZLOG_INFO(log_, "Using SM mode");
      event_mode_ = EVMODE_SM;
      lc = config_.openSectionDefault ("sm");
      smhighprio = config_.getKeyAsDefault<bool>("highprio", true);
      smm_.useHighPrioTP(smhighprio);
    
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

   iofwdutil::ConfigFile csec = config_.openSectionDefault("workqueue");
   workqueue_normal_.reset (new PoolWorkQueue (csec.getKeyAsDefault("minthreadnum", 0),
                                               csec.getKeyAsDefault("maxthreadnum", 100)));
   workqueue_fast_.reset (new SynchronousWorkQueue ());

   taskfactory_.reset (new ThreadTasks (api_.get()));

   taskSMFactory_.reset(new iofwd::tasksm::TaskSMFactory(api_.get(), smm_));
}

DefRequestHandler::~DefRequestHandler ()
{
   /* if this is the state machine mode, shutdown the state machine manager */
   if(event_mode_ == EVMODE_SM)
   {
        /* nothing to do */
   }
   /* if this is the task mode, clear out the work queues before shutdown */
   else if(event_mode_ == EVMODE_TASK)
   {
       /* does not build with icc */
#if 0
        std::vector<WorkItem *> items;
        ZLOG_INFO (log_, "Waiting for normal workqueue to complete all work...");
        workqueue_normal_->waitAll (items);
        for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1));

        items.clear();
        ZLOG_INFO (log_, "Waiting for fast workqueue to complete all work...");
        workqueue_fast_->waitAll (items);
        for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1));
#endif
   }

   api_->finalize();
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
