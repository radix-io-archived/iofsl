#ifndef IOFWD_READTASK_HH
#define IOFWD_READTASK_HH

#include "Task.hh"
#include "ReadRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

class ReadTask : public TaskHelper<ReadRequest>
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
      char * p_buf, uint64_t p_offset, uint64_t p_size);
}; 

}

#endif
