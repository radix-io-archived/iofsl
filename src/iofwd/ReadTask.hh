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

      int ret = api_->read (p.handle,
                            (size_t)p.mem_count, (void**)p.mem_starts, (const size_t*)p.mem_sizes,
                            (size_t)p.file_count, p.file_starts, p.file_sizes);
      request_.setReturnCode (ret);

      std::auto_ptr<iofwdutil::completion::CompletionID> send_id (request_.sendBuffers ());
      send_id->wait ();

      std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
      reply_id->wait ();
   }

}; 

}

#endif
