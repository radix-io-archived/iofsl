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
  SharedCompletionID(const boost::shared_ptr<CompletionID>& ptr)
    : ptr_(ptr) {}
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
  : log_(IOFWDLog::getSource()), exiting(false), async_api_(async_api)
{
  RangeScheduler * rsched;
  const char * sched_algo = getenv("ZOIDFS_SCHED_ALGO");
  if (sched_algo == NULL || strcmp(sched_algo, "fifo") == 0) {
    rsched = new FIFORangeScheduler();
  } else if (strcmp(sched_algo, "merge") == 0) {
    rsched = new MergeRangeScheduler();
  } else {
    assert(false);
  }
  range_sched_.reset(rsched);
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
      bool is_dequeued = range_sched_->dequeue(r);
      if (!is_dequeued)
        continue;
    }
    {
      bool is_merged = !r.child_ranges.empty();
      unsigned int narrays =  is_merged ? r.child_ranges.size() : 1;
      char ** mem_starts = new char*[narrays];
      size_t * mem_sizes = new size_t[narrays];
      uint64_t * file_starts = new uint64_t[narrays];
      uint64_t * file_sizes = new uint64_t[narrays];
      if (is_merged) {
        for (unsigned int i = 0; i < narrays; i++) {
          const Range& child_r = r.child_ranges[i];
          mem_starts[i] = child_r.buf;
          mem_sizes[i] = child_r.en - child_r.st;
          file_starts[i] = child_r.st;
          file_sizes[i] = child_r.en - child_r.st;
        }
      } else {
        mem_starts[0] = r.buf;
        mem_sizes[0] = r.en - r.st;
        file_starts[0] = r.st;
        file_sizes[0] = r.en - r.st;
      }
      assert(r.en > r.st);

      // issue asynchronous I/O using ZoidFSAsyncAPI
      CompletionID *async_id;
      if (r.type == Range::RANGE_WRITE) {
        async_id = async_api_->async_write(
          r.handle, narrays, (const void**)mem_starts, mem_sizes,
          narrays, file_starts, file_sizes);
      } else if (r.type == Range::RANGE_READ) {
        async_id = async_api_->async_read(
          r.handle, narrays, (void**)mem_starts, mem_sizes,
          narrays, file_starts, file_sizes);
      } else {
        assert(false);
      }

      // properly release resources, we wrap async_id by IOCompletionID
      boost::shared_ptr<CompletionID> io_id;
      io_id.reset(new IOCompletionID(async_id, mem_starts, mem_sizes, file_starts, file_sizes));

      // add io_id to associated CompositeCompletionID
      vector<CompositeCompletionID*>& v = r.cids;
      assert(v.size() > 0);
      for (vector<CompositeCompletionID*>::iterator it = v.begin();
           it != v.end(); ++it) {
        CompositeCompletionID * ccid = *it;
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
