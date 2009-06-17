#ifndef IOFWD_WRITETASK_HH
#define IOFWD_WRITETASK_HH

#include "Task.hh"
#include "WriteRequest.hh"
#include "TaskHelper.hh"

namespace iofwd
{

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

      std::auto_ptr<iofwdutil::completion::CompletionID> recv_id (request_.recvBuffers ());
      recv_id->wait ();

      int ret = api_->write (p.handle,
                             (size_t)p.mem_count, (const void **)p.mem_starts, (const size_t*)p.mem_sizes,
                             (size_t)p.file_count, p.file_starts, p.file_sizes);
      request_.setReturnCode (ret);

      std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
      reply_id->wait ();
   }
};

}

#endif
