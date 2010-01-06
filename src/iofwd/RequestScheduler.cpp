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

static void check_ranges(const vector<ChildRange *>& rs)
{
  if (rs.size() >= 2) {
    for (unsigned int i = 1; i < rs.size(); i++) {
      assert(rs[0]->type_ == rs[i]->type_);
      assert(same_handle(rs[0]->handle_, rs[i]->handle_));
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

    ChildRange * r = new ChildRange(file_starts[i], file_starts[i] + file_sizes[i]);
    r->type_ = RANGE_WRITE;
    r->handle_ = handle;
    r->buf_ = (char*)mem_starts[i];
    r->cid_ = ccid;
    r->op_hint_ = op_hint;

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

    ChildRange * r =  new ChildRange(file_starts[i], file_starts[i] + file_sizes[i]);
    r->type_ = RANGE_READ;
    r->handle_ = handle;
    r->buf_ = (char*)mem_starts[i];
    r->cid_ = ccid;
    r->op_hint_ = op_hint;

    boost::mutex::scoped_lock l(lock_);
    range_sched_->enqueue(r);
  }
  notifyConsumer();
  return ccid;
}

void RequestScheduler::run()
{
  vector<ChildRange *> rs;
  const int batch_size = 16;
  int cur_batch = 0;
  while (true) {
    // check if RangeScheduler has pending requests
    ChildRange * tmp_r = NULL;
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
        ChildRange * r = NULL;
        if (range_sched_->empty())
          break;
        bool is_dequeued = range_sched_->dequeue(&r);
        if (!is_dequeued) break;
        if (!rs.empty()) {
          if (r->type_ != rs.back()->type_ || !same_handle(r->handle_, rs.back()->handle_)) {
            // dequeued request is in a different direction/different handle
            // stop batching.
            tmp_r = r;
            has_tmp_r = true;
            break;
          }
        }
    
        /* get size */
        if(r->hasSubIntervals())
        {
            cur_batch += dynamic_cast<ParentRange *>(r)->child_ranges_.size();
        }
        else
        {
            cur_batch += 1;
        }
        rs.push_back(r);
      }
    }

    if (rs.empty())
      continue;
    
    // issue asynchronous I/O for rs
    check_ranges(rs);
    issue(rs);

    //rs.clear();
    /* delete the elements in rs */
    while(!rs.empty())
    {

        /* check if this is a parent */
        if((*rs.begin())->hasSubIntervals())
        {
            ParentRange * r = dynamic_cast<ParentRange *>(*rs.begin());
            rs.erase(rs.begin());
            delete r;
        }
        else
        {
            ChildRange * r = *rs.begin();
            rs.erase(rs.begin());
            delete r;
        }
    }

    if (has_tmp_r) {
      rs.push_back(tmp_r);

      if(tmp_r->hasSubIntervals())
      {
        cur_batch = dynamic_cast<ParentRange *>(tmp_r)->child_ranges_.size();
      }
      else
      {
        cur_batch = 1;
      }
    } else {
      cur_batch = 0;
    }
  }
}

void RequestScheduler::issue(vector<ChildRange *>& rs)
{
  unsigned int narrays = 0;
  for (unsigned int i = 0; i < rs.size(); i++) {
    ChildRange * r = rs[i];
    if(r->hasSubIntervals())
    {
        narrays += dynamic_cast<ParentRange *>(r)->child_ranges_.size();
    }
    else
    {
        narrays += 1;
    }
  }

  char ** mem_starts = new char*[narrays];
  size_t * mem_sizes = new size_t[narrays];
  uint64_t * file_starts = new uint64_t[narrays];
  uint64_t * file_sizes = new uint64_t[narrays];

  unsigned int nth = 0;
  for (unsigned int i = 0; i < rs.size(); i++) {
    ChildRange * r = rs[i];

    bool is_merged = r->hasSubIntervals();
    unsigned int nranges = 1;
    if (is_merged) {
      const ParentRange * pr = dynamic_cast<ParentRange *>(r);
      nranges = pr->child_ranges_.size();
      for(unsigned int j = 0 ; j < nranges ; j++)
      {
        const ChildRange * child_r = pr->child_ranges_[j];
        mem_starts[nth] = child_r->buf_;
        mem_sizes[nth] = child_r->en_ - child_r->st_;
        file_starts[nth] = child_r->st_;
        file_sizes[nth] = child_r->en_ - child_r->st_;
        nth++;
      }
    } else {
      mem_starts[nth] = r->buf_;
      mem_sizes[nth] = r->en_ - r->st_;
      file_starts[nth] = r->st_;
      file_sizes[nth] = r->en_ - r->st_;
      nth++;
    }
    assert(r->en_ > r->st_);
  }
  assert(nth == narrays);

  // issue asynchronous I/O using ZoidFSAsyncAPI
  CompletionID *async_id;
  if (rs[0]->type_ == RANGE_WRITE) {
    async_id = async_api_->async_write(rs[0]->handle_, narrays, (const void**)mem_starts, mem_sizes,
                                       narrays, file_starts, file_sizes, rs[0]->op_hint_);
  } else if (rs[0]->type_ == RANGE_READ) {
    async_id = async_api_->async_read(rs[0]->handle_, narrays, (void**)mem_starts, mem_sizes,
                                      narrays, file_starts, file_sizes, rs[0]->op_hint_);
  } else {
    assert(false);
  }

  // properly release resources, we wrap async_id by IOCompletionID
  boost::shared_ptr<CompletionID> io_id;
  io_id.reset(new IOCompletionID(async_id, mem_starts, mem_sizes, file_starts, file_sizes));

  // add io_id to associated CompositeCompletionID
  for (unsigned int i = 0; i < rs.size(); i++) {
    ChildRange * r = rs[i];
    if(r->hasSubIntervals())
    {
    ParentRange * pr = dynamic_cast<ParentRange *>(r);
    const vector<CompositeCompletionID*>& v = pr->child_cids_;
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
    else
    {
        r->cid_->addCompletionID(new SharedCompletionID(io_id));
    }
  }
}

void RequestScheduler::notifyConsumer()
{
  ready_.notify_all();
}

//===========================================================================
}
