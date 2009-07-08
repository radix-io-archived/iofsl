#include "RequestScheduler.hh"

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <tr1/unordered_map>

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"

#include "iofwdutil/completion/CompletionID.hh"
#include "iofwdutil/completion/CompositeCompletionID.hh"
#include "iofwdutil/completion/WorkQueueCompletionID.hh"
#include "iofwd/WriteRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwd/RangeScheduler.hh"

#include "spatial/Range.hh"
#include "spatial/RangeSet.hh"

using namespace std;
using namespace iofwdutil;
using namespace iofwdutil::completion;
using namespace spatial;

namespace iofwd
{
//===========================================================================

// The class to hold request information
class IOCompletionID : public CompletionID
{
public:
  IOCompletionID(CompletionID * id,
     char ** mem_starts, size_t * mem_sizes,
     uint64_t * file_starts, uint64_t * file_sizes)
    : id_(id), mem_starts_(mem_starts), mem_sizes_(mem_sizes),
      file_starts_(file_starts), file_sizes_(file_sizes)
  {
  }
  virtual ~IOCompletionID() {
    delete id_;
    delete[] mem_starts_;
    delete[] mem_sizes_;
    delete[] file_starts_;
    delete[] file_sizes_;
  }

  virtual void wait() {
    id_->wait();
  }
  virtual bool test(unsigned int maxms) {
    return id_->test(maxms);
  }
private:
  CompletionID * id_;
  char ** mem_starts_;
  size_t * mem_sizes_;
  uint64_t * file_starts_;
  uint64_t * file_sizes_;
};

// The class to share one CompletionID from multiple user
class SharedCompletionID : public CompletionID
{
public:
  SharedCompletionID(boost::shared_ptr<CompletionID> ptr)
    : ptr_(ptr)
  {
    assert(ptr != NULL);
  }
  virtual ~SharedCompletionID() {}

  virtual void wait() {
    ptr_->wait();
  }
  virtual bool test(unsigned int maxms) {
    return ptr_->test(maxms);
  }
private:
  boost::shared_ptr<CompletionID> ptr_;
};

RequestScheduler::RequestScheduler(zoidfs::ZoidFSAsyncAPI * async_api)
  : exiting(false), async_api_(async_api)
{
  range_sched_.reset(new FIFORangeScheduler(this));
  consumethread_.reset(new boost::thread(boost::bind(&RequestScheduler::run, this)));
}

RequestScheduler::~RequestScheduler()
{
  exiting = true;
  ready_.notify_all();
  consumethread_->join();
}

CompletionID * RequestScheduler::enqueueWrite(
  zoidfs::zoidfs_handle_t * handle, size_t count,
  const void ** mem_starts, size_t * mem_sizes,
  uint64_t * file_starts, uint64_t * file_sizes)
{
  CompositeCompletionID * ccid = new CompositeCompletionID(count);

  for (uint32_t i = 0; i < count; i++) {
    assert(mem_sizes[i] == file_sizes[i]);

    Range r;
    r.type = Range::RANGE_WRITE;
    r.handle = handle;
    r.buf = (char*)mem_starts[i];
    r.st = file_starts[i];
    r.en = r.st + file_sizes[i];
    r.cids.push_back(ccid);

    boost::mutex::scoped_lock l(lock_);
    range_sched_->enqueue(r);
  }

  notifyConsumer();

  return ccid;
}

CompletionID * RequestScheduler::enqueueRead(
  zoidfs::zoidfs_handle_t * handle, size_t count,
  void ** mem_starts, size_t * mem_sizes,
  uint64_t * file_starts, uint64_t * file_sizes)
{
  CompositeCompletionID * ccid = new CompositeCompletionID(count);

  for (uint32_t i = 0; i < count; i++) {
    assert(mem_sizes[i] == file_sizes[i]);

    Range r;
    r.type = Range::RANGE_READ;
    r.handle = handle;
    r.buf = (char*)mem_starts[i];
    r.st = file_starts[i];
    r.en = r.st + file_sizes[i];
    r.cids.push_back(ccid);

    boost::mutex::scoped_lock l(lock_);
    range_sched_->enqueue(r);
  }

  notifyConsumer();

  return ccid;
}

void RequestScheduler::run()
{
  while (true) {
    Range r;
    {
      // deque fron scheduler queue
      boost::mutex::scoped_lock l(lock_);
      while (range_sched_->empty() && !exiting) {
        // ready_.timed_wait(l, boost::get_system_time()
        //                      + boost::posix_time::milliseconds(50));
        ready_.wait(l);
      }
      if (exiting)
        break;

      // TODO: dequeue multiple requests
      range_sched_->dequeue(r);
    }
    {
      char ** mem_starts = new char*[1];
      size_t * mem_sizes = new size_t[1];
      mem_starts[0] = r.buf;
      mem_sizes[0] = r.en - r.st;
      uint64_t * file_starts = new uint64_t[1];
      uint64_t * file_sizes = new uint64_t[1];
      file_starts[0] = r.st;
      file_sizes[0] = r.en - r.st;
      assert(r.en > r.st);

      // issue asynchronous I/O using ZoidFSAsyncAPI
      CompletionID *async_id;
      if (r.type == Range::RANGE_WRITE) {
        async_id = async_api_->async_write(
          r.handle, 1, (const void**)mem_starts, mem_sizes,
          1, file_starts, file_sizes);
      } else if (r.type == Range::RANGE_READ) {
        async_id = async_api_->async_read(
          r.handle, 1, (void**)mem_starts, mem_sizes,
          1, file_starts, file_sizes);
      }

      // properly release resources, we wrap async_id by IOCompletionID
      boost::shared_ptr<CompletionID> io_id;
      io_id.reset(new IOCompletionID(async_id, mem_starts, mem_sizes, file_starts, file_sizes));

      // add io_id to associated CompositeCompletionID
      vector<CompositeCompletionID*>& v = r.cids;
      for (unsigned int i = 0; i < v.size(); i++) {
        CompositeCompletionID * ccid = v[i];
        // Because io_id is shared among multiple CompositeCompletionIDs,
        // we use SharedCompletionID to properly release the resource by using
        // boost::shared_ptr (e.g. reference counting).
        ccid->addCompletionID(new SharedCompletionID(io_id));
      }
    }
  }
}

void RequestScheduler::notifyConsumer()
{
  ready_.notify_all();
}

//===========================================================================
}
