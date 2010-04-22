#include "RequestScheduler.hh"

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <tr1/unordered_map>

#include "zoidfs/zoidfs.h"
#include "zoidfs/util/ZoidFSAsyncAPI.hh"

#include "iofwd/WriteRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwd/RangeScheduler.hh"

#include "Range.hh"
#include "RangeSet.hh"

#include "iofwd/tasksm/IOCBWrapper.hh"
#include "iofwd/tasksm/SharedIOCB.hh"

using namespace std;
using namespace iofwdutil;

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

RequestScheduler::RequestScheduler(zoidfs::ZoidFSAsyncAPI * async_api,
      zoidfs::util::ZoidFSAsync * async_cb_api, const iofwdutil::ConfigFile &
      c, int mode)
  : ZoidFSAsyncPT(async_cb_api),
    log_(IOFWDLog::getSource()), exiting_(false), async_api_(async_api),
    async_cb_api_(async_cb_api), mode_(mode), schedActive_(false)
{
  RangeScheduler * rsched;
  char * sched_algo = new char[c.getKeyDefault("schedalgo", "fifo").size() + 1];
  strcpy(sched_algo, c.getKeyDefault("schedalgo", "fifo").c_str());

  if (sched_algo == NULL || strcmp(sched_algo, "fifo") == 0) {
    rsched = new FIFORangeScheduler();
  } else if (strcmp(sched_algo, "merge") == 0) {
    rsched = new MergeRangeScheduler();
  } else {
    assert(false);
  }

  boost::mutex::scoped_lock l(lock_);
  range_sched_.reset(rsched);

#ifndef USE_IOFWD_THREAD_POOL
  consumethread_.reset(new boost::thread(boost::bind(&RequestScheduler::run, this, false)));
#endif

  delete [] sched_algo;
}

RequestScheduler::~RequestScheduler()
{
  {
    boost::mutex::scoped_lock elock(lock_);
    exiting_ = true;
  }

#ifdef USE_IOFWD_THREAD_POOL
  /* wait for the sched to finish */
  while(schedActive_)
  {
        // just spin
  }
#else
  ready_.notify_all();
  consumethread_->join();
#endif

}

void RequestScheduler::write(
  iofwdevent::CBType cb, zoidfs::zoidfs_handle_t * handle, size_t count,
  const void ** mem_starts, size_t * mem_sizes,
  uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint)
{
  // ignore zero-length request
  int valid_count = 0;
  for (uint32_t i = 0; i < count; i++)
    if (file_sizes[i] > 0)
      valid_count++;

  iofwd::tasksm::IOCBWrapper * c = new iofwd::tasksm::IOCBWrapper(cb, valid_count);
  for (uint32_t i = 0; i < count; i++) {
    assert(mem_sizes[i] == file_sizes[i]);
    if (file_sizes[i] == 0) continue;

    ChildRange * r = new ChildRange(file_starts[i], file_starts[i] + file_sizes[i]);
    r->type_ = RANGE_WRITE;
    r->handle_ = handle;
    r->buf_ = (char*)mem_starts[i];
    r->cb_ = c;
    r->op_hint_ = op_hint;

    boost::mutex::scoped_lock l(lock_);
    range_sched_->enqueue(r);

#ifdef USE_IOFWD_THREAD_POOL
    /* if the scheduler is not active, add the scheduler work to the ThreadPool */
    if(!schedActive_)
    {
        iofwdutil::ThreadPool::instance().addWorkUnit(new ReqSchedHelper(this), &ReqSchedHelper::run, iofwdutil::ThreadPool::HIGH, true);
        schedActive_ = true;
    }
#endif
  }
#ifndef USE_IOFWD_THREAD_POOL
  notifyConsumer();
#endif
}

void RequestScheduler::read(
  iofwdevent::CBType cb, zoidfs::zoidfs_handle_t * handle, size_t count,
  void ** mem_starts, size_t * mem_sizes,
  uint64_t * file_starts, uint64_t * file_sizes, zoidfs::zoidfs_op_hint_t * op_hint)
{
  // ignore zero-length request
  int valid_count = 0;
  for (uint32_t i = 0; i < count; i++)
    if (file_sizes[i] > 0)
      valid_count++;

  iofwd::tasksm::IOCBWrapper * c = new iofwd::tasksm::IOCBWrapper(cb, valid_count);
  for (uint32_t i = 0; i < count; i++) {
    assert(mem_sizes[i] == file_sizes[i]);
    if (file_sizes == 0) continue;

    ChildRange * r =  new ChildRange(file_starts[i], file_starts[i] + file_sizes[i]);
    r->type_ = RANGE_READ;
    r->handle_ = handle;
    r->buf_ = (char*)mem_starts[i];
    r->cb_ = c;
    r->op_hint_ = op_hint;

    boost::mutex::scoped_lock l(lock_);
    range_sched_->enqueue(r);

#ifdef USE_IOFWD_THREAD_POOL
    /* if the scheduler is not active, add the scheduler work to the ThreadPool */
    if(!schedActive_)
    {
        iofwdutil::ThreadPool::instance().addWorkUnit(new ReqSchedHelper(this), &ReqSchedHelper::run, iofwdutil::ThreadPool::HIGH, true);
        schedActive_ = true;
    }
#endif
  }
#ifndef USE_IOFWD_THREAD_POOL
  notifyConsumer();
#endif
}

void RequestScheduler::run(bool waitForWork)
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
      while (range_sched_->empty() && !exiting_)
      {
        if(!rs.empty())
            break;
        /* if the scheduler should shutdown when no work is present */
        if(!waitForWork)
        {
            /* update the sched active flag */
            schedActive_ = false;
            return;
        }
        /* else, wait on the condition variable for work */
        else
        {
            ready_.wait(l);
        }
        /* if the exit flag was set, exit this function */
        if(exiting_)
            return;
      }

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
  std::vector< iofwd::tasksm::IOCBWrapper * > cbvec;
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

  int * ret = new int(0);
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
      cbvec.insert(cbvec.end(), pr->child_cbs_.begin(), pr->child_cbs_.end());
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
      cbvec.push_back(r->cb_);
      nth++;
    }
    assert(r->en_ > r->st_);
  }
  assert(nth == narrays);

  /* generate a shared callback wrapper */
  iofwd::tasksm::SharedIOCB * cbs = NULL;
  if(cbvec.size() > 1)
  {
     cbs = new iofwd::tasksm::MultiSharedIOCB(ret, mem_starts, mem_sizes, file_starts, file_sizes);
     dynamic_cast<iofwd::tasksm::MultiSharedIOCB *>(cbs)->loadCBs(cbvec);
  }
  else if(cbvec.size() == 1)
  {
     cbs = new iofwd::tasksm::SingleSharedIOCB(ret, mem_starts, mem_sizes, file_starts, file_sizes);
     dynamic_cast<iofwd::tasksm::SingleSharedIOCB *>(cbs)->loadCB(cbvec[0]);
  }

  const iofwdevent::CBType cbfunc = boost::bind(&iofwd::tasksm::SharedIOCB::issueIOCallbacks, cbs, 0);
  if (rs[0]->type_ == RANGE_WRITE) {
    async_cb_api_->write(cbfunc, ret, rs[0]->handle_, narrays, (const void**)mem_starts, mem_sizes,
                                       narrays, file_starts, file_sizes, rs[0]->op_hint_);
  } else if (rs[0]->type_ == RANGE_READ) {
    async_cb_api_->read(cbfunc, ret, rs[0]->handle_, narrays, (void**)mem_starts, mem_sizes,
                                      narrays, file_starts, file_sizes, rs[0]->op_hint_);
  } else {
    assert(false);
  }
}

void issueWait(int UNUSED(status))
{
   // issue the other callbacks for the request
}

void RequestScheduler::notifyConsumer()
{
    ready_.notify_all();
}

//===========================================================================
}
