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

#include "Range.hh"
#include "RangeSet.hh"

using namespace std;
using namespace iofwdutil;
using namespace iofwdutil::completion;

namespace iofwd
{
//===========================================================================

static bool same_handle(const zoidfs::zoidfs_handle_t *h1,
                        const zoidfs::zoidfs_handle_t *h2)
{
  if (h1 == NULL || h2 == NULL) return false;
  return memcmp(h1->data, h2->data, sizeof(uint8_t)*32) == 0;
}

static void check_ranges(const vector<Range>& rs)
{
  if (rs.size() >= 2) {
    for (unsigned int i = 1; i < rs.size(); i++) {
      assert(rs[0].type == rs[i].type);
      assert(same_handle(rs[0].handle, rs[i].handle));
    }
  }
}

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
  uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint)
{
  // ignore zero-length request
  int valid_count = 0;
  for (uint32_t i = 0; i < count; i++)
    if (file_sizes[i] > 0)
      valid_count++;

  CompositeCompletionID * ccid = new CompositeCompletionID(valid_count);
  for (uint32_t i = 0; i < count; i++) {
    assert(mem_sizes[i] == file_sizes[i]);
    if (file_sizes[i] == 0) continue;

    Range r;
    r.type = Range::RANGE_WRITE;
    r.handle = handle;
    r.buf = (char*)mem_starts[i];
    r.st = file_starts[i];
    r.en = r.st + file_sizes[i];
    r.cids.push_back(ccid);
    r.op_hint = op_hint;

    boost::mutex::scoped_lock l(lock_);
    range_sched_->enqueue(r);
  }
  notifyConsumer();
  return ccid;
}

CompletionID * RequestScheduler::enqueueRead(
  zoidfs::zoidfs_handle_t * handle, size_t count,
  void ** mem_starts, size_t * mem_sizes,
  uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint)
{
  // ignore zero-length request
  int valid_count = 0;
  for (uint32_t i = 0; i < count; i++)
    if (file_sizes[i] > 0)
      valid_count++;
  
  CompositeCompletionID * ccid = new CompositeCompletionID(valid_count);
  for (uint32_t i = 0; i < count; i++) {
    assert(mem_sizes[i] == file_sizes[i]);
    if (file_sizes == 0) continue;

    Range r;
    r.type = Range::RANGE_READ;
    r.handle = handle;
    r.buf = (char*)mem_starts[i];
    r.st = file_starts[i];
    r.en = r.st + file_sizes[i];
    r.cids.push_back(ccid);
    r.op_hint = op_hint;

    boost::mutex::scoped_lock l(lock_);
    range_sched_->enqueue(r);
  }
  notifyConsumer();
  return ccid;
}

void RequestScheduler::run()
{
  vector<Range> rs;
  const int batch_size = 16;
  int cur_batch = 0;
  while (true) {
    // check if RangeScheduler has pending requests
    Range tmp_r;
    bool has_tmp_r = false;
    {
      boost::mutex::scoped_lock l(lock_);
      while (range_sched_->empty() && !exiting) {
        if (!rs.empty()) break;
        ready_.wait(l);
      }
      if (exiting)
        break;

      // dequeue requests of same direction (read/write), same handle
      // TODO: batch_size should be tunable
      while (cur_batch < batch_size) {
        Range r;
        if (range_sched_->empty())
          break;
        bool is_dequeued = range_sched_->dequeue(r);
        if (!is_dequeued) break;
        if (!rs.empty()) {
          if (r.type != rs.back().type || !same_handle(r.handle, rs.back().handle)) {
            // dequeued request is in a different direction/different handle
            // stop batching.
            tmp_r = r;
            has_tmp_r = true;
            break;
          }
        }
        cur_batch += r.child_ranges.empty() ? 1 : r.child_ranges.size();
        rs.push_back(r);
      }
    }

    if (rs.empty())
      continue;
    
    // issue asynchronous I/O for rs
    check_ranges(rs);
    issue(rs);

    rs.clear();
    if (has_tmp_r) {
      rs.push_back(tmp_r);
      cur_batch = tmp_r.child_ranges.empty() ? 1 : tmp_r.child_ranges.size();
    } else {
      cur_batch = 0;
    }
  }
}

void RequestScheduler::issue(const vector<Range>& rs)
{
  unsigned int narrays = 0;
  for (unsigned int i = 0; i < rs.size(); i++) {
    const Range& r = rs[i];
    narrays += r.child_ranges.empty() ? 1 : r.child_ranges.size();
  }

  char ** mem_starts = new char*[narrays];
  size_t * mem_sizes = new size_t[narrays];
  uint64_t * file_starts = new uint64_t[narrays];
  uint64_t * file_sizes = new uint64_t[narrays];

  unsigned int nth = 0;
  for (unsigned int i = 0; i < rs.size(); i++) {
    const Range& r = rs[i];

    bool is_merged = !r.child_ranges.empty();
    unsigned int nranges = is_merged ? r.child_ranges.size() : 1;
    if (is_merged) {
      for (unsigned int j = 0; j < nranges; j++) {
        const Range& child_r = r.child_ranges[j];
        mem_starts[nth] = child_r.buf;
        mem_sizes[nth] = child_r.en - child_r.st;
        file_starts[nth] = child_r.st;
        file_sizes[nth] = child_r.en - child_r.st;
        nth++;
      }
    } else {
      mem_starts[nth] = r.buf;
      mem_sizes[nth] = r.en - r.st;
      file_starts[nth] = r.st;
      file_sizes[nth] = r.en - r.st;
      nth++;
    }
    assert(r.en > r.st);
  }
  assert(nth == narrays);

  // issue asynchronous I/O using ZoidFSAsyncAPI
  CompletionID *async_id;
  if (rs[0].type == Range::RANGE_WRITE) {
    async_id = async_api_->async_write(rs[0].handle, narrays, (const void**)mem_starts, mem_sizes,
                                       narrays, file_starts, file_sizes, rs[0].op_hint);
  } else if (rs[0].type == Range::RANGE_READ) {
    async_id = async_api_->async_read(rs[0].handle, narrays, (void**)mem_starts, mem_sizes,
                                      narrays, file_starts, file_sizes, rs[0].op_hint);
  } else {
    assert(false);
  }

  // properly release resources, we wrap async_id by IOCompletionID
  boost::shared_ptr<CompletionID> io_id;
  io_id.reset(new IOCompletionID(async_id, mem_starts, mem_sizes, file_starts, file_sizes));

  // add io_id to associated CompositeCompletionID
  for (unsigned int i = 0; i < rs.size(); i++) {
    const vector<CompositeCompletionID*>& v = rs[i].cids;
    assert(v.size() > 0);
    for (vector<CompositeCompletionID*>::const_iterator it = v.begin();
         it != v.end(); ++it) {
      CompositeCompletionID * ccid = *it;
      // Because io_id is shared among multiple CompositeCompletionIDs,
      // we use SharedCompletionID to properly release the resource by using
      // boost::shared_ptr (e.g. reference counting).
      ccid->addCompletionID(new SharedCompletionID(io_id));
    }
  }
}

void RequestScheduler::notifyConsumer()
{
  ready_.notify_all();
}

//===========================================================================
}
