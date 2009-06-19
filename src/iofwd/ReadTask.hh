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

      // p.mem_sizes is uint32_t array, but ZoidFSAPI::write() takes size_t array
      // for its arguments. Therefore, in (sizeof(size_t) != sizeof(uint32_t))
      // environment (64bit), p.mem_sizes is not valid for size_t array.
      // We allocate temporary buffer to fix this problem.
      size_t * tmp_mem_sizes = (size_t*)p.mem_sizes;
      bool need_size_t_workaround = (sizeof(size_t) != sizeof(uint32_t));
      if (need_size_t_workaround) {
         tmp_mem_sizes = new size_t[p.mem_count];
         for (uint32_t i = 0; i < p.mem_count; i++)
            tmp_mem_sizes[i] = p.mem_sizes[i];
      }

      int ret = api_->read (p.handle,
                            (size_t)p.mem_count, (void**)p.mem_starts, tmp_mem_sizes,
                            (size_t)p.file_count, p.file_starts, p.file_sizes);
      request_.setReturnCode (ret);

      if (need_size_t_workaround)
         delete[] tmp_mem_sizes;

      std::auto_ptr<iofwdutil::completion::CompletionID> send_id (request_.sendBuffers ());
      send_id->wait ();

      std::auto_ptr<iofwdutil::completion::CompletionID> reply_id (request_.reply ());
      reply_id->wait ();
   }

}; 

}

#endif
