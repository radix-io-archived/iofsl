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
  RangeScheduler() {}
  virtual ~RangeScheduler() {}
  virtual void enqueue(const spatial::Range& r) = 0;
  virtual bool empty() = 0;
  virtual bool dequeue(spatial::Range& r) = 0;
protected:
  RequestScheduler * sched_;
};

// Simple First-In-First-Out range scheduler
class FIFORangeScheduler : public RangeScheduler
{
public:
  FIFORangeScheduler() {}
  virtual ~FIFORangeScheduler() { q_.clear(); }
  virtual void enqueue(const spatial::Range& r);
  virtual bool empty();
  virtual bool dequeue(spatial::Range& r);
protected:
  std::deque<spatial::Range> q_;
};

// Handle-Based-Merge range scheduler
struct HandleQueue;
class MergeRangeScheduler : public RangeScheduler
{
public:
  MergeRangeScheduler() : bytes_queued(0) {}
  virtual ~MergeRangeScheduler() {}
  virtual void enqueue(const spatial::Range& r);
  virtual bool empty();
  virtual bool dequeue(spatial::Range& r);

private:
  void io_enqueue(const spatial::Range &r);
  bool io_dequeue(spatial::Range &r);

  void deadline_enqueue(const spatial::Range &r);
  bool deadline_dequeue(spatial::Range &r);
  
private:
  std::deque<HandleQueue*> q_;
  std::tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*> rm_;
  std::tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*> wm_;

  ssize_t bytes_queued;
};

//===========================================================================
}

#endif
