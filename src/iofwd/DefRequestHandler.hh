#ifndef IOFWD_DEFREQUESTHANDLER_HH
#define IOFWD_DEFREQUESTHANDLER_HH

#include "iofwdutil/IOFWDLog.hh"

#include "iofwd/service/Service.hh"
#include "iofwd/RequestHandler.hh"
#include "iofwd/ThreadTasks.hh"
#include "zoidfs/util/LogAPI.hh"
#include "iofwdutil/ConfigFile.hh"
#include "zoidfs/util/ZoidFSDefAsync.hh"
#include "sm/SMManager.hh"
#include "iofwd/tasksm/TaskSMFactory.hh"

#include <memory>
#include <vector>
#include <boost/scoped_ptr.hpp>


namespace iofwdutil
{
   namespace workqueue
   {
      // forward
      class WorkQueue;
      class WorkItem;
   }
}

namespace iofwd
{

   // Service forwards
   class Log;
   class Config;

/**
 * This class accepts requests from the frontend and uses a workqueue to run
 * the requests until the request returns false (indicating it is done)
 */
class DefRequestHandler : public RequestHandler,
                          public service::Service
{
public:

   /// Accept requests and put them on the workqueue
   virtual void handleRequest (int count, Request ** reqs);

   DefRequestHandler (service::ServiceManager & man);

   virtual ~DefRequestHandler ();

protected:
   boost::shared_ptr<Log> log_service_;
   boost::shared_ptr<Config> config_service_;

   iofwdutil::IOFWDLogSource & log_;

   boost::scoped_ptr<iofwdutil::workqueue::WorkQueue> workqueue_normal_;
   boost::scoped_ptr<iofwdutil::workqueue::WorkQueue> workqueue_fast_;

   /// Holds completed requests until they are freed
   std::vector<iofwdutil::workqueue::WorkItem *> completed_;

   /// Associates request with a task
   boost::scoped_ptr<ThreadTasks> taskfactory_;

   // state machine factory
   boost::scoped_ptr<iofwd::tasksm::TaskSMFactory> taskSMFactory_;

   /// Async API
   boost::scoped_ptr<zoidfs::util::ZoidFSAsync> api_;

   sm::SMManager smm_;

   enum{EVMODE_TASK = 0, EVMODE_SM};
   int event_mode_;

   iofwdutil::ThreadPool & tp_;
};

}

#endif
