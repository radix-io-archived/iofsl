#ifndef IOFWD_DEFREQUESTHANDLER_HH
#define IOFWD_DEFREQUESTHANDLER_HH

#include <memory>
#include <vector>
#include "iofwdutil/IOFWDLog.hh"
#include "iofwd/RequestHandler.hh"
#include "iofwd/BMIBufferPool.hh"
#include "iofwd/ThreadTasks.hh"
#include "iofwd/TaskPool.hh"
#include "zoidfs/util/LogAPI.hh"
#include "iofwdutil/ConfigFile.hh"

namespace iofwdutil
{
   namespace workqueue
   {
      // forward
      class WorkQueue;
      class WorkItem;
   }
}

namespace zoidfs
{
   class ZoidFSAsyncAPI;
}

namespace iofwd
{
   class RequestScheduler;
}

namespace iofwd
{

/**
 * This class accepts requests from the frontend and uses a workqueue to run
 * the requests until the request returns false (indicating it is done)
 *
 * Requests are able to reschedule themselves if needed.
 */
class DefRequestHandler : public RequestHandler
{
public:

   /// Accept requests and put them on the workqueue
   virtual void handleRequest (int count, Request ** reqs);

   DefRequestHandler (const iofwdutil::ConfigFile & c);

   virtual ~DefRequestHandler ();

protected:
   void reschedule (Task * t);

protected:
   iofwdutil::IOFWDLogSource & log_;

   std::auto_ptr<iofwdutil::workqueue::WorkQueue> workqueue_normal_;
   std::auto_ptr<iofwdutil::workqueue::WorkQueue> workqueue_fast_;

   /// Holds completed requests until they are freed
   std::vector<iofwdutil::workqueue::WorkItem *> completed_;

   /// Associates request with a task
   std::auto_ptr<ThreadTasks> taskfactory_;

   /// API
   zoidfs::LogAPI api_;
   zoidfs::ZoidFSAsyncAPI * async_api_;

   /// Scheduler
   RequestScheduler * sched_;

   /// BufferPool
   BMIBufferPool * bpool_;

   // TaskPool
   TaskPool * tpool_;

   // config file
   const iofwdutil::ConfigFile & config_;
};

}

#endif
