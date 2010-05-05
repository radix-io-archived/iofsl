#ifndef IOFWD_RANGE_HH
#define IOFWD_RANGE_HH

#include <stdlib.h>
#include <vector>
#include <list>
#include <iostream>

#include "zoidfs/zoidfs.h"
#include "iofwdevent/CBType.hh"
#include "iofwd/tasksm/IOCBWrapper.hh"

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

//===========================================================================

/*
Range classes for the IntervalTree...

The child range only needs data specific to the child (io type, start, end, handle, etc...)

The parent range contains the child properties and data structures for storing children / interval subranges

*/
class ChildRange
{
    public:
        ChildRange() : buf_(NULL), st_(0), en_(0), cb_(NULL)
        {
        }

        ChildRange(uint64_t st, uint64_t en) : st_(st), en_(en), cb_(NULL)
        {
        }

        virtual ~ChildRange()
        {
        }

        virtual bool hasSubIntervals()
        {
            return false;
        }

        RangeType type_;
        const zoidfs::zoidfs_handle_t * handle_;
        char * buf_;
        uint64_t st_;
        uint64_t en_;
        zoidfs::zoidfs_op_hint_t * op_hint_;
        iofwd::tasksm::IOCBWrapper * cb_;
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

        virtual ~ParentRange()
        {
            /* clear the CB vector */
            child_cbs_.clear();

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
                fprintf(stderr, "%s: st_ = %i > c->st_ = %i\n", __func__, st_, c->st_);
                st_ = c->st_;
            }

            if(en_ < c->en_)
            {
                fprintf(stderr, "%s: en_ = %i > c->en_ = %i\n", __func__, en_, c->en_);
                en_ = c->en_;
            }

            /* copy the child ranges and cids */
            insertMultipleChildren(c->child_ranges_);
            insertMultipleCBs(c->child_cbs_);

            return true;
        }

        bool add(ChildRange * c)
        {
            /* update the parent interval */
            if(st_ > c->st_)
            {
                fprintf(stderr, "%s: st_ = %i > c->st_ = %i\n", __func__, st_, c->st_);
                st_ = c->st_;
            }

            if(en_ < c->en_)
            {
                fprintf(stderr, "%s: en_ = %i > c->en_ = %i\n", __func__, en_, c->en_);
                en_ = c->en_;
            }

            /* copy the child ranges and cids */
            insertSingleChild(c);
            insertSingleCB(c->cb_);

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

        void insertSingleCB(iofwd::tasksm::IOCBWrapper * cb)
        {
            child_cbs_.push_back(cb);
        }

        void insertMultipleChildren(std::vector<ChildRange *> & r)
        {
            child_ranges_.insert(child_ranges_.end(), r.begin(), r.end());
        }

        void insertMultipleCBs(std::vector<iofwd::tasksm::IOCBWrapper *> & c)
        {
            child_cbs_.insert(child_cbs_.end(), c.begin(), c.end());
        }

        /* this is the vector of sub ranges */
        std::vector<ChildRange *> child_ranges_;

        /* this is the vector of cbs for the subranges */
        std::vector<iofwd::tasksm::IOCBWrapper *> child_cbs_;
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
