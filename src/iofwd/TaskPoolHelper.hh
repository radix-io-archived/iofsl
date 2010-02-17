#ifndef IOFWD_TASKPOOLHELPER_HH
#define IOFWD_TASKPOOLHELPER_HH

#include "Task.hh"
#include "TaskHelper.hh"
#include "iofwdutil/completion/BMIResource.hh"
#include "BMIBufferPool.hh"

#include <boost/function.hpp>

namespace zoidfs
{
    class ZoidFSAPI;
    class ZoidFSAsyncAPI;
}

namespace iofwd
{

class RequestScheduler;
class BufferPool;

/**
 * Helper class for Threaded tasks.
 */
template <typename T>
class TaskPoolHelper : public Task
{
   public:
      /**
       * The task takes ownership of the request
       */
      TaskPoolHelper (ThreadTaskParam & param)
         : Task (param.resched), request_ (static_cast<T *> (param.req)),
           api_ (param.api), async_api_(param.async_api), sched_(param.sched),
           bpool_(param.bpool), bmi_ (param.bmi), full_init_(true), pool_allocate_(false)
      {
#ifndef NDEBUG
         // This will throw if the request is not of the expected type
         dynamic_cast<T *> (param.req);
#endif
        setTaskAllocType(true);
      }

      /* construtor for preallocating a task and later parameterizing it */
      TaskPoolHelper(iofwdutil::completion::BMIResource & bmi, boost::function<void (Task *)> reschedule, boost::function<void (int, Task *)> addToPool)
        : Task(reschedule), request_(NULL), api_(NULL), async_api_(NULL), sched_(NULL),
          bpool_(NULL), bmi_(bmi), full_init_(false), pool_allocate_(true), addToPool_(addToPool)
      {
        setTaskAllocType(true);
      }

      T * getRequest ()
      { return request_; }


      ~TaskPoolHelper ()
      {
         // The task owns the request and needs to destroy it

         /* only delete the task if it was init */
         if(full_init_)
         {
            delete (request_);
            request_ = NULL;
         }
      }

    void resetTaskHelper(ThreadTaskParam & param)
    {
         /* cleanup old request */
         if(full_init_)
         {
            delete (request_);
         }

         /* this is a full init */
         full_init_ = true;

         /* set new request params */
         request_ = static_cast<T *>(param.req);
         api_ = param.api;
         async_api_ = param.async_api;
         sched_ = param.sched;
         bpool_ = param.bpool;
    }

    virtual void cleanup()
    {
         int opid = request_->getOpID();
         if(full_init_)
         {
             delete request_;
             request_ = NULL;
             full_init_ = false;
         }
         addToPool_(opid, this);
    }

   protected:
      T * request_;

      zoidfs::ZoidFSAPI * api_;
      zoidfs::ZoidFSAsyncAPI * async_api_;
      RequestScheduler * sched_;
      BMIBufferPool * bpool_;
      iofwdutil::completion::BMIResource & bmi_;

      bool full_init_;
      bool pool_allocate_;

      boost::function<void (int, Task *)> addToPool_;
};

}

#endif
