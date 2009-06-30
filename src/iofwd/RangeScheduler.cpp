#include "RangeScheduler.hh"
#include "RequestScheduler.hh"

#include <cassert>

using namespace spatial;
using namespace iofwd;

namespace iofwd
{
//===========================================================================

void FIFORangeScheduler::enqueue(const Range& r)
{
  q_.push_back(r);
  sched_->notifyConsumer();
}

bool FIFORangeScheduler::empty()
{
  return q_.empty();
}

void FIFORangeScheduler::dequeue(Range& r)
{
  assert(!q_.empty());
  r = q_.front();
  q_.pop_front();
}

//===========================================================================

void MergeRangeScheduler::enqueue(const Range& r)
{
  if (m_.find(r.handle) == m_.end()) {
    RangeSet * rs = new RangeSet();
    rs->add(r);

    q_.push_back(r.handle);
    m_[r.handle] = rs;
  } else {
    RangeSet * rs = m_.find(r.handle)->second;
    rs->add(r);
  }

  sched_->notifyConsumer();
}

bool MergeRangeScheduler::empty()
{
  return q_.empty();
}

void MergeRangeScheduler::dequeue(Range& r)
{
  assert(!q_.empty());

  const zoidfs::zoidfs_handle_t * handle = q_.front();
  assert(m_.find(handle) != m_.end());
  RangeSet * s = m_[handle];
  s->pop_front(r);
  if (s->empty()) {
    m_.erase(handle);
    delete s;
  }

  q_.pop_front();
  q_.push_back(handle);
}

//===========================================================================
}
