#ifndef __IOFWD_TASKSM_TASKSMFACTORY_HH__
#define __IOFWD_TASKSM_TASKSMFACTORY_HH__

#include <boost/function.hpp>
#include "iofwd/BMIMemoryManager.hh"
#include "sm/SMManager.hh"
#include "iofwd/Request.hh"

namespace zoidfs
{
    class ZoidFSAPI;
    class ZoidFSAsyncAPI;

    namespace util
    {
        class ZoidFSAsync;
    }
}

namespace iofwd
{
    class Request;
    class RequestScheduler;

    namespace tasksm
    {
//===========================================================================


/**
 * Task factory that generates task which block until complete.
 */
class TaskSMFactory
{
public:

   TaskSMFactory(RequestScheduler * sched,
         sm::SMManager & smm, zoidfs::util::ZoidFSAsync * api)
      : sched_(sched), smm_(smm), api_(api)
   {
   }

   ~TaskSMFactory()
   {
   }

   void operator () (iofwd::Request * req);

protected:
   RequestScheduler * sched_;
   sm::SMManager & smm_;
   zoidfs::util::ZoidFSAsync * api_;
};

    }
//===========================================================================
}

#endif
