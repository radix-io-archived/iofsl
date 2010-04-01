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

void FIFORangeScheduler::enqueue(ChildRange * r)
{
  q_.push_back(r);
}

bool FIFORangeScheduler::empty()
{
  return q_.empty();
}

bool FIFORangeScheduler::dequeue(ChildRange ** r)
{
  if (q_.empty()) return false;
  *r = q_.front();
  q_.pop_front();
  return true;
}

//===========================================================================

#define DEFAULT_QUANTUM 8

struct HandleQueue
{
  RangeType type;
  const zoidfs::zoidfs_handle_t * handle;
  HierarchicalRangeSet * hrs;
  IntervalTreeRangeSet * itrs;
  int quantum;
};

void MergeRangeScheduler::io_enqueue(ChildRange * r)
{
  tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*>& m_ = (r->type_ == RANGE_READ) ? rm_ : wm_;
  if (m_.find(r->handle_) == m_.end()) {
    // if no HandleQueue exists for r.handle, create new one
    HandleQueue * hq = new HandleQueue();
    hq->type = r->type_;
    hq->handle = r->handle_;
    hq->quantum = DEFAULT_QUANTUM;

#ifdef ITRS
    IntervalTreeRangeSet * itrs = new IntervalTreeRangeSet();
    itrs->add(r);
    hq->itrs = itrs;
#else
    HierarchicalRangeSet * hrs = new HierarchicalRangeSet();
    hrs->add(r);
    hq->hrs = hrs;
#endif /* #ifdef ITRS */

    q_.push_back(hq);
    m_[hq->handle] = hq;
  } else {
    // if corresponding HandleQueue exists for r.handle,
    // add to its RangeSet.
    HandleQueue * hq = m_.find(r->handle_)->second;

#ifdef ITRS
    IntervalTreeRangeSet * itrs = hq->itrs;
    itrs->add(r);
#else
    HierarchicalRangeSet * hrs = hq->hrs;
    hrs->add(r);
#endif /* #ifdef ITRS */

  }
}

bool MergeRangeScheduler::io_dequeue(ChildRange ** r)
{
  if (q_.empty()) return false;

  HandleQueue * hq = q_.front();
  tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*>& m_ = (hq->type == RANGE_READ) ? rm_ : wm_;
  assert(m_.find(hq->handle) != m_.end());

#ifdef ITRS
  IntervalTreeRangeSet * itrs = hq->itrs;
  assert(!itrs->empty());
  itrs->pop_front(r);
#else
  HierarchicalRangeSet * hrs = hq->hrs;
  assert(!hrs->empty());
  hrs->pop_front(r);
  //fprintf(stderr, "pop_front = %p\n", *r);
#endif /* #ifdef ITRS */

  hq->quantum--;
#ifdef ITRS
  if (hq->quantum > 0 && !itrs->empty()) {
#else
  if (hq->quantum > 0 && !hrs->empty()) {
#endif /* #ifdef ITRS */
    // do nothing
#ifdef ITRS
  } else if (hq->quantum == 0 && !itrs->empty()) {
#else
  } else if (hq->quantum == 0 && !hrs->empty()) {
#endif /* #ifdef ITRS */
    // turn to another queue
    hq->quantum = DEFAULT_QUANTUM;
    q_.pop_front();
    q_.push_back(hq);
  } else {
    // delete queue
    q_.pop_front();
    m_.erase(hq->handle);
#ifdef ITRS
    delete hq->itrs;
#else
    delete hq->hrs;
#endif /* #ifdef ITRS */
    delete hq;    
  }

  return true;
}

void MergeRangeScheduler::deadline_enqueue(ChildRange * UNUSED(r))
{
  // TODO: implement
  return;
}

bool MergeRangeScheduler::deadline_dequeue(ChildRange ** UNUSED(r))
{
  // TODO: implement
  return false;
}

void MergeRangeScheduler::enqueue(ChildRange * r)
{
  deadline_enqueue(r);
  io_enqueue(r);

  // update statistics
  bytes_queued += r->en_ - r->st_;
}

bool MergeRangeScheduler::empty()
{
  return q_.empty();
}

bool MergeRangeScheduler::dequeue(ChildRange ** r)
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
    bytes_queued -= ((*r)->en_ - (*r)->st_);
  
  return is_dequeued;
}

//===========================================================================
}
