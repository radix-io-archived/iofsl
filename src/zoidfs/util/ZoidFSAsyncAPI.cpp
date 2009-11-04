#include "ZoidFSAsyncAPI.hh"

#include <boost/format.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/lambda/construct.hpp>

#include "zoidfs-util.hh"
#include "ZoidFSAPI.hh"
#include "iofwdutil/completion/CompletionID.hh"
#include "iofwdutil/workqueue/WorkItem.hh"
#include "iofwdutil/workqueue/WorkQueue.hh"
#include "iofwdutil/tools.hh"

using namespace boost;
using namespace boost::lambda;

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
      uint64_t * file_sizes,
      zoidfs_op_hint_t * op_hint)
      : api_(api), handle_(handle),
        mem_count_(mem_count), mem_starts_(mem_starts), mem_sizes_(mem_sizes),
        file_count_(file_count), file_starts_(file_starts), file_sizes_(file_sizes), op_hint_(op_hint)
   {}
   virtual ~AsyncWriteTask() {};
   void doWork() {
      api_->write (handle_, mem_count_, mem_starts_, mem_sizes_,
         file_count_, file_starts_, file_sizes_, op_hint_);
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
   zoidfs_op_hint_t * op_hint_;
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
      uint64_t * file_sizes,
      zoidfs_op_hint_t * op_hint)
      : api_(api), handle_(handle),
        mem_count_(mem_count), mem_starts_(mem_starts), mem_sizes_(mem_sizes),
        file_count_(file_count), file_starts_(file_starts), file_sizes_(file_sizes), op_hint_(op_hint)
   {}
   virtual ~AsyncReadTask() {};
   void doWork() {
      api_->read (handle_, mem_count_, mem_starts_, mem_sizes_,
         file_count_, file_starts_, file_sizes_, op_hint_);
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
   zoidfs_op_hint_t * op_hint_;
};

}

ZoidFSAsyncAPI::~ZoidFSAsyncAPI()
{
   if (q_ != NULL) {
      std::vector<iofwdutil::workqueue::WorkItem *> items; 
      q_->waitAll (items);
      for_each (items.begin(), items.end(), boost::lambda::bind(delete_ptr(), boost::lambda::_1)); 
      delete q_;
   }
}

iofwdutil::completion::CompletionID * ZoidFSAsyncAPI::async_write(
   const zoidfs_handle_t * handle,
   size_t mem_count,
   const void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const uint64_t file_starts[],
   uint64_t file_sizes[],
   zoidfs_op_hint_t * op_hint)
{
   AsyncWriteTask * task = new AsyncWriteTask (api_, handle,
      mem_count, mem_starts, mem_sizes, file_count, file_starts, file_sizes, op_hint);
   return q_->queueWork (task);
}

iofwdutil::completion::CompletionID * ZoidFSAsyncAPI::async_read(
   const zoidfs_handle_t * handle,
   size_t mem_count,
   void * mem_starts[],
   const size_t mem_sizes[],
   size_t file_count,
   const uint64_t file_starts[],
   uint64_t file_sizes[],
   zoidfs_op_hint_t * op_hint)
{
   AsyncReadTask * task = new AsyncReadTask (api_, handle,
      mem_count, mem_starts, mem_sizes, file_count, file_starts, file_sizes, op_hint);
   return q_->queueWork (task);
}

//===========================================================================
}
