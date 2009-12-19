#ifndef IOFWD_RANGE_HH
#define IOFWD_RANGE_HH

#include <stdlib.h>
#include <vector>

#include "zoidfs/zoidfs.h"

namespace iofwdutil
{
   namespace completion
   {
      class CompositeCompletionID;
   }
}

namespace iofwd
{
//===========================================================================

/* forward declarations */
class ChildRange;
class ParentRange;

enum RangeType
{
    RANGE_WRITE = 0,
    RANGE_READ
};

class Range
{
public:
  Range() : buf(NULL), st(0), en(0) {}
  Range(uint64_t st, uint64_t en) : st(st), en(en) {}

  Range(ChildRange c);
  Range(ParentRange p);

  RangeType type;
  zoidfs::zoidfs_handle_t * handle;
  char * buf;
  uint64_t st;
  uint64_t en;
  zoidfs::zoidfs_op_hint_t * op_hint;

  // If this Range is composed of some ranges, this variable
  // holds child ranges.
  std::vector<Range> child_ranges;

  // CompositeCompletionIDs which tests/waits for this range
  // TODO:
  //   need parameterized by templates or something
  std::vector<iofwdutil::completion::CompositeCompletionID*> cids;
};

inline bool operator==(const Range& r1, const Range& r2)
{
  return (r1.st == r2.st) && (r1.en == r2.en);
}

inline bool operator<(const Range& r1, const Range& r2)
{
  if (r1.st == r2.st)
    return r1.en < r2.en;
  return r1.st < r2.st;
}

//===========================================================================

/*
Range classes for the IntervalTree...

The child range only needs data specific to the child (io type, start, end, handle, etc...)

The parent range contains the child properties and data structures for storing children / interval subranges

TODO integrate these classes with the original request merging code
*/
class ChildRange
{
    public:
        ChildRange() : buf_(NULL), st_(0), en_(0), cid_(NULL)
        {
        }

        ChildRange(uint64_t st, uint64_t en) : st_(st), en_(en)
        {
        }

        ~ChildRange()
        {
        }

        Range operator() (ChildRange c)
        {
            Range r;

            r.type = c.type_;
            r.handle = c.handle_;
            r.buf = c.buf_;
            r.st = c.st_;
            r.en = c.en_;
            r.op_hint = c.op_hint_;
            r.cids.push_back(c.cid_);

            return r;
        }

        RangeType type_;
        zoidfs::zoidfs_handle_t * handle_;
        char * buf_;
        uint64_t st_;
        uint64_t en_;
        zoidfs::zoidfs_op_hint_t * op_hint_;
        iofwdutil::completion::CompositeCompletionID * cid_;
    
};

class ParentRange : public ChildRange
{
    public:
        ParentRange() : ChildRange()
        {
        }

        ParentRange(uint64_t st, uint64_t en) : ChildRange(st, en)
        {
        }

        ~ParentRange()
        {
            /* clear the vectors */
            child_ranges_.clear();
            child_cids_.clear();
        }

        /* add the children from c to this parent */
        bool merge(ParentRange & c)
        {
            /* update the parent interval */
            if(st_ > c.st_)
            {
                st_ = c.st_; 
            }
    
            if(en_ < c.en_)
            {
                en_ = c.en_;
            }

            /* copy the child ranges and cids */
            child_ranges_.insert(child_ranges_.begin(), c.child_ranges_.begin(), c.child_ranges_.end());
            child_cids_.insert(child_cids_.begin(), c.child_cids_.begin(), c.child_cids_.end());

            return true;
        }

        bool add(ChildRange & c)
        {
            /* update the parent interval */
            if(st_ > c.st_)
            {
                st_ = c.st_; 
            }
    
            if(en_ < c.en_)
            {
                en_ = c.en_;
            }

            /* copy the child ranges and cids */
            child_ranges_.insert(child_ranges_.begin(), c);
            child_cids_.insert(child_cids_.begin(), c.cid_);

            return true;
        }

        /* convert the ParentRange to a Range */
        Range operator() (ParentRange p)
        {
            Range r;
            r.st = p.st_;
            r.en = p.en_;
            r.buf = p.buf_;
            r.op_hint = p.op_hint_;
            r.handle = p.handle_;
            r.type = p.type_;

            r.child_ranges.clear();
            r.cids.clear();

            if(!child_ranges_.empty())
            {
                r.child_ranges.insert(r.child_ranges.begin(), p.child_ranges_.begin(), p.child_ranges_.end());
            }

            if(!child_cids_.empty())
            {
                r.cids.insert(r.cids.begin(), p.child_cids_.begin(), p.child_cids_.end());
            }

            return r;
        }

        /* this is the vector of sub ranges */
        std::vector<ChildRange> child_ranges_;

        /* this is the vector of cids for the subranges */
        std::vector<iofwdutil::completion::CompositeCompletionID*> child_cids_;
};

inline bool operator==(const ChildRange& r1, const ChildRange& r2)
{
  return (r1.st_ == r2.st_) && (r1.en_ == r2.en_);
}

inline bool operator<(const ChildRange& r1, const ChildRange& r2)
{
  if (r1.st_ == r2.st_)
    return r1.en_ < r2.en_;
  return r1.st_ < r2.st_;
}

} /* using namespace iofwd */
#endif
