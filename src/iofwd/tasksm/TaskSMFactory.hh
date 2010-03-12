#ifndef __IOFWD_TASKSM_TASKSMFACTORY_HH__
#define __IOFWD_TASKSM_TASKSMFACTORY_HH__

#include <boost/function.hpp>
#include "iofwd/BMIBufferPool.hh"
#include "sm/SMManager.hh"
#include "iofwd/Request.hh"

namespace zoidfs
{
   class ZoidFSAPI;
   class ZoidFSAsyncAPI;
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
         BMIBufferPool * bpool, sm::SMManager & smm)
      : sched_(sched), bpool_(bpool), smm_(smm)
   {
   }

   ~TaskSMFactory()
   {
   }

   void operator () (iofwd::Request * req);

protected:
   RequestScheduler * sched_;
   BMIBufferPool * bpool_;
   sm::SMManager & smm_;
};

    }
//===========================================================================
}

#endif
