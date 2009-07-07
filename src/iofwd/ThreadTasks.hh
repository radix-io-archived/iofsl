#ifndef IOFWD_THREADTASKS_HH
#define IOFWD_THREADTASKS_HH

#include <boost/function.hpp>
#include "iofwdutil/completion/BMIResource.hh"

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
         BufferPool * pool)
      : reschedule_(resched), api_(api), async_api_(async_api), sched_(sched),
        pool_(pool)
   {
   }

   Task * operator () (Request * req); 

protected:
   boost::function<void (Task *)> reschedule_; 

   zoidfs::ZoidFSAPI * api_;
   zoidfs::ZoidFSAsyncAPI * async_api_;
   RequestScheduler * sched_;
   BufferPool * pool_;

   iofwdutil::completion::ContextBase ctx_; 
   iofwdutil::completion::BMIResource bmi_; 
}; 

//===========================================================================
}

#endif
