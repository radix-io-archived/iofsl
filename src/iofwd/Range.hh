#ifndef IOFWD_RANGE_HH
#define IOFWD_RANGE_HH

#include <stdlib.h>
#include <vector>
#include <list>
#include <iostream>

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

        virtual bool hasSubIntervals()
        {
            return false;
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
            /* clear the cid vector */
            child_cids_.clear();
       
            /* cleanup the children */ 
            destroySubRanges();
        }

        bool destroySubRanges()
        {
            /* delete all of the sub intervals */
            while(!child_ranges_.empty())
            {
                ChildRange * r = *child_ranges_.begin();
                child_ranges_.erase(child_ranges_.begin());
                delete r;
            }
            return true;
        }

        virtual bool hasSubIntervals()
        {
            return true;
        }

        /* add the children from c to this parent */
        bool merge(ParentRange * c)
        {
            /* update the parent interval */
            if(st_ > c->st_)
            {
                st_ = c->st_; 
            }
    
            if(en_ < c->en_)
            {
                en_ = c->en_;
            }

            /* copy the child ranges and cids */
            insertMultipleChildren(c->child_ranges_);
            insertMultipleCids(c->child_cids_);

            return true;
        }

        bool add(ChildRange * c)
        {
            /* update the parent interval */
            if(st_ > c->st_)
            {
                st_ = c->st_; 
            }
    
            if(en_ < c->en_)
            {
                en_ = c->en_;
            }

            /* copy the child ranges and cids */
            insertSingleChild(c);
            insertSingleCid(c->cid_);

            return true;
        }

        /* wrappers that use vector methods with decent performance
         * use vector push_back for single items
         * use vector insert at end() for multiple items
         */
        void insertSingleChild(ChildRange * r)
        {
            child_ranges_.push_back(r);
        }

        void insertSingleCid(iofwdutil::completion::CompositeCompletionID * c)
        {
            child_cids_.push_back(c);
        }

        void insertMultipleChildren(std::vector<ChildRange *> & r)
        {
            child_ranges_.insert(child_ranges_.end(), r.begin(), r.end());
        }

        void insertMultipleCids(std::vector<iofwdutil::completion::CompositeCompletionID*> & c)
        {
            child_cids_.insert(child_cids_.end(), c.begin(), c.end());
        }

        /* this is the vector of sub ranges */
        std::vector<ChildRange *> child_ranges_;

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

inline bool operator==(const ParentRange& r1, const ParentRange& r2)
{
  return (r1.st_ == r2.st_) && (r1.en_ == r2.en_);
}

inline bool operator<(const ParentRange& r1, const ParentRange& r2)
{
  if (r1.st_ == r2.st_)
    return r1.en_ < r2.en_;
  return r1.st_ < r2.st_;
}

} /* using namespace iofwd */
#endif
