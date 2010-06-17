#ifndef IOFWD_RANGESCHEDULER_HH
#define IOFWD_RANGESCHEDULER_HH

#include <tr1/unordered_map>
#include <deque>
#include <boost/scoped_ptr.hpp>

#include "iofwd/Range.hh"
#include "iofwd/BaseRangeSet.hh"
#include "iofwd/HierarchicalRangeSet.hh"
#include "iofwd/IntervalTreeRangeSet.hh"
#include "iofwdutil/Factory.hh"
#include "iofwdutil/Configurable.hh"
#include "zoidfs/zoidfs.h"
#include "iofwdutil/ConfigFile.hh"
#include "iofwdutil/IOFWDLog.hh"

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
  MergeRangeScheduler(const iofwdutil::ConfigFile rqsection) 
        : bytes_queued(0), default_quantum_(0), log_(iofwdutil::IOFWDLog::getSource("mergerangescheduler"))
  {
        /* get the quantum */
        default_quantum_ = rqsection.getKeyAsDefault("defquantum", 8);
        rname_ = rqsection.getKeyDefault("merger", "hierarchical");

        ZLOG_INFO(log_, iofwdutil::format("Using request merger '%s' with quantum '%i'") % rname_ % default_quantum_);

        /* store the factory */
        range_factory_ = boost::function< BaseRangeSet*() >(iofwdutil::Factory<std::string, iofwd::BaseRangeSet>::construct(rname_));
  }

  virtual ~MergeRangeScheduler() {}
  virtual void enqueue(ChildRange * r);
  virtual bool empty();
  virtual bool dequeue(ChildRange ** r);

private:
  void io_enqueue(ChildRange * r);
  bool io_dequeue(ChildRange ** r);

  void deadline_enqueue(ChildRange * r);
  bool deadline_dequeue(ChildRange ** r);
  
  iofwd::BaseRangeSet * createRangeSet()
  {
        return dynamic_cast<iofwd::BaseRangeSet *>(range_factory_());
  }

private:
  std::deque<HandleQueue*> q_;
  std::tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*> rm_;
  std::tr1::unordered_map<const zoidfs::zoidfs_handle_t*, HandleQueue*> wm_;

  ssize_t bytes_queued;

  int default_quantum_;

  iofwdutil::IOFWDLogSource & log_;

  boost::function< BaseRangeSet*() > range_factory_;
  std::string rname_;
};

//===========================================================================
}

#endif
