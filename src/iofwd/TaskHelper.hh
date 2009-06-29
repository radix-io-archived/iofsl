#ifndef IOFWD_TASKHELPER_HH
#define IOFWD_TASKHELPER_HH

#include "Task.hh"
#include "iofwdutil/completion/BMIResource.hh"

namespace zoidfs
{
   class ZoidFSAPI;
   class ZoidFSAsyncAPI;
}

namespace iofwd
{

class RequestScheduler;

class ThreadTaskParam
{
public:
   Request   *                          req;
   boost::function<void (Task*)>        resched; 
   zoidfs::ZoidFSAPI *                  api;
   zoidfs::ZoidFSAsyncAPI *             async_api;
   RequestScheduler *                   sched;
   iofwdutil::completion::BMIResource & bmi; 

   ThreadTaskParam (Request * r, 
      boost::function<void (Task*)> r2,
      zoidfs::ZoidFSAPI * a1,
      zoidfs::ZoidFSAsyncAPI * a2,
      RequestScheduler * s,
      iofwdutil::completion::BMIResource & b)
      : req(r), bmi(b), api(a1), async_api(a2), sched(s), resched(r2)
   {
   }

} ; 

template <typename T>

/**
 * Helper class for Threaded tasks.
 */
class TaskHelper : public Task
{
   public:
      /**
       * The task takes ownership of the request
       */
      TaskHelper (ThreadTaskParam & param)
         : Task (param.resched), request_ (static_cast<T &> (*param.req)), 
           api_ (param.api), async_api_(param.async_api), sched_(param.sched),
           bmi_ (param.bmi)
      {
#ifndef NDEBUG
         // This will throw if the request is not of the expected type
         dynamic_cast<T &> (*param.req); 
#endif
      }

      T * getRequest ()
      { return &request_; }

      ~TaskHelper ()
      {
         // The task owns the request and needs to destroy it
         delete (&request_); 
      }

   protected:
      T & request_; 

      zoidfs::ZoidFSAPI *                  api_;
      zoidfs::ZoidFSAsyncAPI *             async_api_;
      RequestScheduler * sched_;
      iofwdutil::completion::BMIResource & bmi_; 
}; 



}

#endif
