#ifndef IOFWD_WRITETASK_HH
#define IOFWD_WRITETASK_HH

#include "Task.hh"
#include "WriteRequest.hh"
#ifdef USE_IOFWD_TASK_POOL
#include "iofwd/TaskPoolHelper.hh"
#else
#include "iofwd/TaskHelper.hh"
#endif
#include <boost/function.hpp>
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

struct RetrievedBuffer;
#ifdef USE_IOFWD_TASK_POOL
class WriteTask : public TaskPoolHelper<WriteRequest>, public iofwdutil::InjectPool<WriteTask>
{
public:
   WriteTask (ThreadTaskParam & p)
      : TaskPoolHelper<WriteRequest>(p)
#else
class WriteTask : public TaskHelper<WriteRequest>, public iofwdutil::InjectPool<WriteTask>
{
public:
   WriteTask (ThreadTaskParam & p)
      : TaskHelper<WriteRequest>(p)
#endif
   {
   }

#ifdef USE_IOFWD_TASK_POOL
   WriteTask(iofwdutil::completion::BMIResource & bmi, boost::function<void (Task *)> reschedule, boost::function<void (int, Task *)> addToPool) : TaskHelper<WriteRequest>(bmi, reschedule, addToPool)
   {
   }
#endif

   virtual ~WriteTask()
   {
   }

#ifdef USE_IOFWD_TASK_POOL
   void resetTask(ThreadTaskParam & p)
   {
        resetTaskHelper(p);
   }
#endif

   void run ()
   {
      // parameter decode
#ifdef USE_IOFWD_TASK_POOL
      const WriteRequest::ReqParam & p = request_->decodeParam ();
#else
      const WriteRequest::ReqParam & p = request_.decodeParam ();
#endif
      if (p.pipeline_size == 0)
         runNormalMode(p);
      else
         runPipelineMode(p);
   }

private:
   void runNormalMode(const WriteRequest::ReqParam & p);

   void runPipelineMode(const WriteRequest::ReqParam & p);
   iofwdutil::completion::CompletionID * execPipelineIO(const WriteRequest::ReqParam & p,
      RetrievedBuffer * b);
};

}

#endif
