#ifndef IOFWD_RANGESCHEDULER_HH
#define IOFWD_RANGESCHEDULER_HH

#include <tr1/unordered_map>
#include <deque>

#include "iofwd/Range.hh"
#include "iofwd/BaseRangeSet.hh"
#include "iofwd/HierarchialRangeSet.hh"
#include "iofwd/IntervalTreeRangeSet.hh"
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
  virtual void enqueue(ChildRange * r) = 0;
  virtual bool empty() = 0;
  virtual bool dequeue(ChildRange ** r) = 0;
protected:
  RequestScheduler * sched_;
};

// Simple First-In-First-Out range scheduler
class FIFORangeScheduler : public RangeScheduler
{
public:
  FIFORangeScheduler() {}
  virtual ~FIFORangeScheduler() { q_.clear(); }
  virtual void enqueue(ChildRange * r);
  virtual bool empty();
  virtual bool dequeue(ChildRange ** r);
protected:
  std::deque<ChildRange *> q_;
};

// Handle-Based-Merge range scheduler
struct HandleQueue;
class MergeRangeScheduler : public RangeScheduler
{
public:
  MergeRangeScheduler(int default_quantum) : bytes_queued(0), default_quantum_(default_quantum) {}
  virtual ~MergeRangeScheduler() {}
  virtual void enqueue(ChildRange * r);
  virtual bool empty();
  virtual bool dequeue(ChildRange ** r);

private:
  void io_enqueue(ChildRange * r);
  bool io_dequeue(ChildRange ** r);

  void deadline_enqueue(ChildRange * r);
  bool deadline_dequeue(ChildRange ** r);
  
private:
  std::deque<HandleQueue*> q_;
  std::tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*> rm_;
  std::tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*> wm_;

  ssize_t bytes_queued;

  int default_quantum_;
};

//===========================================================================
}

#endif
