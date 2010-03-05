#ifndef IOFWD_READTASK_HH
#define IOFWD_READTASK_HH

#include "Task.hh"
#include "ReadRequest.hh"
#include "TaskHelper.hh"
#include "iofwdutil/InjectPool.hh"

namespace iofwd
{

struct ReadBuffer;

class ReadTask : public TaskHelper<ReadRequest>, public iofwdutil::InjectPool<ReadTask>
{
public:
   ReadTask (ThreadTaskParam & p)
      : TaskHelper<ReadRequest>(p)
   {
   }
   virtual ~ReadTask()
   {
   }

   void run ()
   {
      // parameter decode
      const ReadRequest::ReqParam & p = request_.decodeParam ();
      if (p.pipeline_size == 0)
         runNormalMode(p);
      else
         runPipelineMode(p);
   }

private:
   void runNormalMode(const ReadRequest::ReqParam & p);

   void runPipelineMode(const ReadRequest::ReqParam & p);
   iofwdutil::completion::CompletionID * execPipelineIO(const ReadRequest::ReqParam & p,
      ReadBuffer * b);
};

}

#endif
