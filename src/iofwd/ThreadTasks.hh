#ifndef IOFWD_THREADTASKS_HH
#define IOFWD_THREADTASKS_HH

#include <boost/function.hpp>
#include "iofwdutil/completion/BMIResource.hh"
#include "iofwd/BMIBufferPool.hh"

namespace zoidfs
{
   class ZoidFSAPI;
   class ZoidFSAsyncAPI;
}

namespace iofwd
{
//===========================================================================

class Task;
class Request; 
class RequestScheduler;
class BufferPool;

/**
 * Task factory that generates task which block until complete.
 */
class ThreadTasks
{
public:

   ThreadTasks (boost::function<void (Task *)> & resched,
         zoidfs::ZoidFSAPI * api,
         zoidfs::ZoidFSAsyncAPI * async_api,
         RequestScheduler * sched,
         BMIBufferPool * bpool)
      : reschedule_(resched), api_(api), async_api_(async_api), sched_(sched),
        bpool_(bpool)
   {
   }

   Task * operator () (Request * req); 

protected:
   boost::function<void (Task *)> reschedule_; 

   zoidfs::ZoidFSAPI * api_;
   zoidfs::ZoidFSAsyncAPI * async_api_;
   RequestScheduler * sched_;
   BMIBufferPool * bpool_;

   iofwdutil::completion::ContextBase ctx_; 
   iofwdutil::completion::BMIResource bmi_; 
}; 

//===========================================================================
}

#endif
