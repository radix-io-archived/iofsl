#ifndef IOFWD_WRITETASK_HH
#define IOFWD_WRITETASK_HH

#include "Task.hh"
#include "WriteRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

struct RetrievedBuffer;

class WriteTask : public TaskHelper<WriteRequest>
{
public:
   WriteTask (ThreadTaskParam & p)
      : TaskHelper<WriteRequest>(p)
   {
   }

   virtual ~WriteTask()
   {
   }

   void run ()
   {
      // parameter decode
      const WriteRequest::ReqParam & p = request_.decodeParam ();
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
