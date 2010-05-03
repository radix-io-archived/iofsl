
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>

// @TODO: boost 1.43 has unordered containers
#include <tr1/unordered_map>

#include "RequestScheduler.hh"

#include "zoidfs/zoidfs.h"

#include "iofwd/WriteRequest.hh"
#include "iofwd/ReadRequest.hh"
#include "iofwd/RangeScheduler.hh"

#include "iofwd/Range.hh"
#include "iofwd/BaseRangeSet.hh"

// @TODO: the following 2 tasksm headers are useable outside of tasksm (they
// are used here) and should be moved up one level
#include "iofwd/tasksm/IOCBWrapper.hh"
#include "iofwd/tasksm/SharedIOCB.hh"
#include "iofwdutil/IOFWDLog.hh"
#include "iofwdutil/LinkHelper.hh"

using namespace std;
using namespace iofwdutil;
using namespace zoidfs;


GENERIC_FACTORY_CLIENT(std::string,
      zoidfs::util::ZoidFSAsync,
      iofwd::RequestScheduler,
      "requestscheduler",
      requestscheduler);

namespace iofwd
{
//===========================================================================

// @TODO: use src/zoidfs/util/zoidfs-ops.hh : implements operator == and so on
// for zoidfs structures.
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

RequestScheduler::RequestScheduler ()
    : log_(IOFWDLog::getSource("requestscheduler")), exiting_(false),
       schedActive_(false), batch_size_(0)
{
}

void RequestScheduler::configure (const iofwdutil::ConfigFile & config)
{
   boost::mutex::scoped_lock l(lock_);

   const std::string api (config.getKeyDefault ("api", "defasync"));
   ZLOG_INFO(log_, format("Using async API '%s'") % api);
   api_.reset (iofwdutil::Factory<
         std::string,
         zoidfs::util::ZoidFSAsync>::construct(api)());
   iofwdutil::Configurable::configure_if_needed (api_.get(),
         config.openSectionDefault(api.c_str()));

   // For ZoidFSAsyncPT
   setAsyncPT (api_.get());

   /* get the scheduler type... defaults to FIFO */
   const std::string schedalgo = config.getKeyDefault("schedalgo", "fifo");
   if (schedalgo == "fifo")
   {
      ZLOG_INFO(log_, "Using FIFO scheduler");
      range_sched_.reset(new FIFORangeScheduler());
   }
   else if (schedalgo == "merge")
   {
      ZLOG_INFO(log_, "Using MERGE scheduler");
      range_sched_.reset(new MergeRangeScheduler(config.openSectionDefault ("rangesched") ));
   }
   else
   {
      ZLOG_ERROR(log_, format("Unknown scheduler specified! ('%s')")
            % schedalgo);
      // @TODO: need configuration file exception
      throw "bad config";
   }

   /* get the scheduler batch size */
   batch_size_ = config.getKeyAsDefault("batchsize", 16);
}

RequestScheduler::~RequestScheduler()
{
  {
    boost::mutex::scoped_lock elock(lock_);
    exiting_ = true;
  }

  /* wait for the sched to finish */
  while(schedActive_)
  {
        // just spin
  }
}

void RequestScheduler::write (const iofwdevent::CBType & cb, int * ret, const
      zoidfs::zoidfs_handle_t * handle, size_t count, const void *
      mem_starts[], const size_t * mem_sizes, size_t file_count, const
      zoidfs::zoidfs_file_ofs_t file_starts[], const
      zoidfs::zoidfs_file_size_t file_sizes[], zoidfs::zoidfs_op_hint_t *
      op_hint)
{
   // @TODO Note: the Requestscheduler depends on this. Needs to be fixed.
   // @TODO Need to do proper error (return code) handling! ... current ret is UNUSED
   ALWAYS_ASSERT(file_count == count);

  // ignore zero-length request
  int valid_count = 0;
  for (size_t i = 0; i < count; i++)
    if (file_sizes[i] > 0)
      valid_count++;

  iofwd::tasksm::IOCBWrapper * c = new iofwd::tasksm::IOCBWrapper(cb, valid_count, ret);
  for (size_t i = 0; i < count; i++) {
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

    /* if the scheduler is not active, add the scheduler work to the ThreadPool */
    if(!schedActive_)
    {
        iofwdutil::ThreadPool::instance().addWorkUnit(new ReqSchedHelper(this), &ReqSchedHelper::run, iofwdutil::ThreadPool::HIGH, true);
        schedActive_ = true;
    }
  }
}

void RequestScheduler::read (const iofwdevent::CBType & cb, int * ret, const
      zoidfs::zoidfs_handle_t * handle, size_t count, void * mem_starts[],
      const size_t * mem_sizes, size_t file_count,
      const zoidfs::zoidfs_file_ofs_t file_starts[],
      const zoidfs::zoidfs_file_size_t file_sizes[], zoidfs::zoidfs_op_hint_t * op_hint)
{
   // @TODO Need to do proper error (return code) handling! ... current ret is UNUSED
   ALWAYS_ASSERT(file_count == count);
  // ignore zero-length request
  int valid_count = 0;
  for (size_t i = 0; i < count; i++)
    if (file_sizes[i] > 0)
      valid_count++;

  iofwd::tasksm::IOCBWrapper * c = new iofwd::tasksm::IOCBWrapper(cb, valid_count, ret);
  for (size_t i = 0; i < count; i++) {
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

    /* if the scheduler is not active, add the scheduler work to the ThreadPool */
    if(!schedActive_)
    {
        iofwdutil::ThreadPool::instance().addWorkUnit(new ReqSchedHelper(this), &ReqSchedHelper::run, iofwdutil::ThreadPool::HIGH, true);
        schedActive_ = true;
    }
  }
}

void RequestScheduler::run(bool waitForWork)
{
  vector<ChildRange *> rs;
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
        /* if the exit flag was set, exit this function */
        if(exiting_)
            return;
      }

      // dequeue requests of same direction (read/write), same handle
      while (cur_batch < batch_size_) {
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
  zoidfs_file_ofs_t * file_starts = new zoidfs_file_ofs_t[narrays];
  zoidfs_file_size_t * file_sizes = new zoidfs_file_size_t[narrays];

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
    api_->write(cbfunc, ret, rs[0]->handle_, narrays, (const void**)mem_starts, mem_sizes,
                                       narrays, file_starts, file_sizes, rs[0]->op_hint_);
  } else if (rs[0]->type_ == RANGE_READ) {
    api_->read(cbfunc, ret, rs[0]->handle_, narrays, (void**)mem_starts, mem_sizes,
                                      narrays, file_starts, file_sizes, rs[0]->op_hint_);
  } else {
    assert(false);
  }
}

void issueWait(int UNUSED(status))
{
   // issue the other callbacks for the request
}

//===========================================================================
}
