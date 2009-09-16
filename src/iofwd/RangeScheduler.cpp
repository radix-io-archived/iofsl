#include "RangeScheduler.hh"
#include "RequestScheduler.hh"
#include "iofwdutil/tools.hh"

#include <iostream>
#include <cassert>

using namespace std;
using namespace iofwd;

namespace iofwd
{
//===========================================================================

void FIFORangeScheduler::enqueue(const Range& r)
{
  q_.push_back(r);
}

bool FIFORangeScheduler::empty()
{
  return q_.empty();
}

bool FIFORangeScheduler::dequeue(Range& r)
{
  if (q_.empty()) return false;
  r = q_.front();
  q_.pop_front();
  return true;
}

//===========================================================================

#define DEFAULT_QUANTUM 8

struct HandleQueue
{
  Range::RangeType type;
  const zoidfs::zoidfs_handle_t * handle;
  RangeSet * rs;
  int quantum;
};

void MergeRangeScheduler::io_enqueue(const Range &r)
{
  tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*>& m_ = (r.type == Range::RANGE_READ) ? rm_ : wm_;
  if (m_.find(r.handle) == m_.end()) {
    // if no HandleQueue exists for r.handle, create new one
    HandleQueue * hq = new HandleQueue();
    hq->type = r.type;
    hq->handle = r.handle;
    hq->quantum = DEFAULT_QUANTUM;

    RangeSet * rs = new RangeSet();
    rs->add(r);
    hq->rs = rs;

    q_.push_back(hq);
    m_[hq->handle] = hq;
  } else {
    // if corresponding HandleQueue exists for r.handle,
    // add to its RangeSet.
    HandleQueue * hq = m_.find(r.handle)->second;
    RangeSet * rs = hq->rs;
    rs->add(r);
  }
}

bool MergeRangeScheduler::io_dequeue(Range &r)
{
  if (q_.empty()) return false;

  HandleQueue * hq = q_.front();
  tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*>& m_ = (hq->type == Range::RANGE_READ) ? rm_ : wm_;
  assert(m_.find(hq->handle) != m_.end());

  RangeSet * rs = hq->rs;
  assert(!rs->empty());
  rs->pop_front(r);

  hq->quantum--;
  if (hq->quantum > 0 && !rs->empty()) {
    // do nothing
  } else if (hq->quantum == 0 && !rs->empty()) {
    // turn to another queue
    hq->quantum = DEFAULT_QUANTUM;
    q_.pop_front();
    q_.push_back(hq);
  } else {
    // delete queue
    q_.pop_front();
    m_.erase(hq->handle);
    delete hq->rs;
    delete hq;    
  }

  return true;
}

void MergeRangeScheduler::deadline_enqueue(const Range& UNUSED(r))
{
  // TODO: implement
  return;
}

bool MergeRangeScheduler::deadline_dequeue(Range & UNUSED(r))
{
  // TODO: implement
  return false;
}

void MergeRangeScheduler::enqueue(const Range& r)
{
  deadline_enqueue(r);
  io_enqueue(r);

  // update statistics
  bytes_queued += r.en - r.st;
}

bool MergeRangeScheduler::empty()
{
  return q_.empty();
}

bool MergeRangeScheduler::dequeue(Range& r)
{
  bool is_dequeued = false;

  // from deadline queue
  if (!is_dequeued)
    is_dequeued = deadline_dequeue(r);

  // from I/O queue
  if (!is_dequeued)
    is_dequeued = io_dequeue(r);

  // update statistics
  if (is_dequeued)
    bytes_queued -= (r.en - r.st);
  
  return is_dequeued;
}

//===========================================================================
}
