#ifndef IOFWD_THREADTASKS_HH
#define IOFWD_THREADTASKS_HH

#include <boost/function.hpp>
#include "iofwdutil/completion/BMIResource.hh"
#include "iofwd/BMIMemoryManager.hh"

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

/**
 * Task factory that generates task which block until complete.
 */
class ThreadTasks
{
public:

   ThreadTasks (
         zoidfs::ZoidFSAPI * api,
         zoidfs::ZoidFSAsyncAPI * async_api,
         RequestScheduler * sched)
      : api_(api), async_api_(async_api), sched_(sched)
   {
   }

   Task * operator () (Request * req); 

protected:
   zoidfs::ZoidFSAPI * api_;
   zoidfs::ZoidFSAsyncAPI * async_api_;
   RequestScheduler * sched_;

   iofwdutil::completion::ContextBase ctx_; 
   iofwdutil::completion::BMIResource bmi_; 
}; 

//===========================================================================
}

#endif
