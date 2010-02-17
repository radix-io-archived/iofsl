#ifndef IOFWD_WRITETASK_HH
#define IOFWD_WRITETASK_HH

#include "Task.hh"
#include "WriteRequest.hh"
#include "iofwd/TaskPoolHelper.hh"
#include <boost/function.hpp>

namespace iofwd
{

struct RetrievedBuffer;

class WriteTask : public TaskPoolHelper<WriteRequest>
{
public:
   WriteTask (ThreadTaskParam & p)
      : TaskPoolHelper<WriteRequest>(p)
   {
   }

   WriteTask(iofwdutil::completion::BMIResource & bmi, boost::function<void (Task *)> reschedule, boost::function<void (int, Task *)> addToPool) : TaskPoolHelper<WriteRequest>(bmi, reschedule, addToPool)
   {
   }

   virtual ~WriteTask()
   {
   }

   void resetTask(ThreadTaskParam & p)
   {
        resetTaskHelper(p);
   }

   void run ()
   {
      // parameter decode
      const WriteRequest::ReqParam & p = request_->decodeParam ();
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
