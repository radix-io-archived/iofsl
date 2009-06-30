#ifndef IOFWD_RANGESCHEDULER_HH
#define IOFWD_RANGESCHEDULER_HH

#include <tr1/unordered_map>
#include <deque>

#include "spatial/Range.hh"
#include "spatial/RangeSet.hh"
#include "zoidfs/zoidfs.h"

namespace iofwd
{
//===========================================================================

class RequestScheduler;

// Abstract class to reorder/merge incoming ranges
class RangeScheduler
{
public:
  RangeScheduler(RequestScheduler * sched_)
    : sched_(sched_) {}
  virtual ~RangeScheduler() {}
  virtual void enqueue(const spatial::Range& r) = 0;
  virtual bool empty() = 0;
  virtual void dequeue(spatial::Range& r) = 0;
protected:
  RequestScheduler * sched_;
};

// Simple First-In-First-Out range scheduler
class FIFORangeScheduler : public RangeScheduler
{
public:
  FIFORangeScheduler(RequestScheduler * sched_)
    : RangeScheduler(sched_) {}
  virtual ~FIFORangeScheduler() { q_.clear(); }
  virtual void enqueue(const spatial::Range& r);
  virtual bool empty();
  virtual void dequeue(spatial::Range& r);
protected:
  std::deque<spatial::Range> q_;
};

// Handle-Based-Merge range scheduler
class MergeRangeScheduler : public RangeScheduler
{
public:
  MergeRangeScheduler(RequestScheduler * sched_)
    : RangeScheduler(sched_) {}
  virtual ~MergeRangeScheduler() {}
  virtual void enqueue(const spatial::Range& r);
  virtual bool empty();
  virtual void dequeue(spatial::Range& r);
private:
  std::deque<const zoidfs::zoidfs_handle_t*> q_;
  std::tr1::unordered_map<const zoidfs::zoidfs_handle_t*, spatial::RangeSet*> m_;
};

//===========================================================================
}

#endif
