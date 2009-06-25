#include "ZoidFSAsyncAPI.hh"

#include <boost/format.hpp>

#include "zoidfs-util.hh"
#include "ZoidFSAPI.hh"
#include "iofwdutil/completion/CompletionID.hh"
#include "iofwdutil/workqueue/WorkItem.hh"
#include "iofwdutil/workqueue/WorkQueue.hh"
#include "iofwdutil/tools.hh"

using namespace boost; 

namespace zoidfs
{
//===========================================================================

namespace {

class AsyncWriteTask : public iofwdutil::workqueue::WorkItem
{
public:
   AsyncWriteTask(ZoidFSAPI * api,
      const zoidfs_handle_t * handle,
      size_t mem_count,
      const void ** mem_starts,
      const size_t * mem_sizes,
      size_t file_count,
      const uint64_t * file_starts,
      uint64_t * file_sizes)
      : api_(api), handle_(handle),
        mem_count_(mem_count), mem_starts_(mem_starts), mem_sizes_(mem_sizes),
        file_count_(file_count), file_starts_(file_starts), file_sizes_(file_sizes)
   {}
   virtual ~AsyncWriteTask() {};
   void doWork() {
      api_->write (handle_, mem_count_, mem_starts_, mem_sizes_,
         file_count_, file_starts_, file_sizes_);
   }
private:
   ZoidFSAPI * api_;
   const zoidfs_handle_t * handle_;
   size_t mem_count_;
   const void ** mem_starts_;
   const size_t * mem_sizes_;
   size_t file_count_;
   const uint64_t * file_starts_;
   uint64_t * file_sizes_;   
};

class AsyncReadTask : public iofwdutil::workqueue::WorkItem
{
public:
   AsyncReadTask(ZoidFSAPI * api,
      const zoidfs_handle_t * handle,
      size_t mem_count,
      void ** mem_starts,
      const size_t * mem_sizes,
      size_t file_count,
      const uint64_t * file_starts,
      uint64_t * file_sizes)
      : api_(api), handle_(handle),
        mem_count_(mem_count), mem_starts_(mem_starts), mem_sizes_(mem_sizes),
        file_count_(file_count), file_starts_(file_starts), file_sizes_(file_sizes)
   {}
   virtual ~AsyncReadTask() {};
   void doWork() {
      api_->read (handle_, mem_count_, mem_starts_, mem_sizes_,
         file_count_, file_starts_, file_sizes_);
   }
private:
   ZoidFSAPI * api_;
   const zoidfs_handle_t * handle_;
   size_t mem_count_;
   void ** mem_starts_;
   const size_t * mem_sizes_;
   size_t file_count_;
   const uint64_t * file_starts_;
   uint64_t * file_sizes_;   
};

}

iofwdutil::completion::CompletionID * ZoidFSAsyncAPI::async_write(
   const zoidfs_handle_t * handle,
   size_t mem_count,
   const void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const uint64_t file_starts[],
   uint64_t file_sizes[])
{
   AsyncWriteTask * task = new AsyncWriteTask (api_, handle,
      mem_count, mem_starts, mem_sizes, file_count, file_starts, file_sizes);
   return q_->queueWork (task);
}

iofwdutil::completion::CompletionID * ZoidFSAsyncAPI::async_read(
   const zoidfs_handle_t * handle,
   size_t mem_count,
   void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const uint64_t file_starts[],
   uint64_t file_sizes[])
{
   AsyncReadTask * task = new AsyncReadTask (api_, handle,
      mem_count, mem_starts, mem_sizes, file_count, file_starts, file_sizes);
   return q_->queueWork (task);
}

//===========================================================================
}
