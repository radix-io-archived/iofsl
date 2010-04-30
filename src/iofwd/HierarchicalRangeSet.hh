#ifndef IOFWD_RANGESET_HH
#define IOFWD_RANGESET_HH

#include <cstdio>
#include <iostream>
#include <cassert>
#include <set>
#include <map>

#include "iofwd/Range.hh"
#include "iofwd/BaseRangeSet.hh"

#include "iofwdutil/LinkHelper.hh"
#include "iofwdutil/Configurable.hh"

namespace iofwd
{
//===========================================================================


class HierarchicalRangeSet : public BaseRangeSet
{
public:
    HierarchicalRangeSet()
    {
    }

    ~HierarchicalRangeSet()
    {
        /* clear the range set */
        while(!ranges.empty())
        {
            if(dynamic_cast<ChildRange *>(*ranges.begin())->hasSubIntervals())
            {
                ParentRange * r = dynamic_cast<ParentRange *>(*ranges.begin());
                ranges.erase(ranges.begin());
                delete r;
            }
            else
            {
                ChildRange * r = dynamic_cast<ChildRange *>(*ranges.begin());
                ranges.erase(ranges.begin());
                delete r;
            }
        }

        /* clear the maps */
        st_map.clear();
        en_map.clear();
    }

    size_t size() const
    {
        return ranges.size();
    }

    bool empty() const
    {
        return ranges.empty();
    }

    void get(ChildRange ** r)
    {
        assert(!ranges.empty());
        *r = *ranges.begin();
        st_map.erase((*r)->st_);
        en_map.erase((*r)->en_);
        ranges.erase(ranges.begin());
    }

    /* find buffer intersects... no dup ranges are returned */
    void intersect(const ChildRange * r, std::set<ChildRange *> & s)
    {
        std::map<uint64_t, ChildRange *>::iterator it;

        /*
            The beginning part of rr overlaps the end of r
            r  [st .... en]
            rr      [st .... en]
        */
        it = st_map.lower_bound(r->st_);
        while (it != st_map.end())
        {
            ChildRange * rr = it->second;
            if (r->st_ <= rr->st_ && rr->st_ <= r->en_)
            {
                s.insert(rr);
            }
            else
            {
                break;
            }
            ++it;
        }

        /*
            The last part of rr overlaps the beginning of r
            r        [st .... en]
            rr [st .... en]
        */
        it = en_map.lower_bound(r->st_);
        while (it != en_map.end())
        {
            ChildRange * rr = it->second;
            if (r->st_ <= rr->en_ && rr->en_ <= r->en_)
            {
                s.insert(rr);
            }
            else
            {
                break;
            }
            ++it;
        }
    }

    /* identify all of the buffers that are a subset of existing buffers */
    void included(const ChildRange * r, std::set<ChildRange *> & s)
    {
        std::map<uint64_t, ChildRange *>::iterator it;

        /*
            Buffer rr overlaps all of r
            r        [st .... en]
            rr [st ................... en]
        */
        it = en_map.lower_bound(r->en_);
        while (it != en_map.end())
        {
            ChildRange * rr = it->second;
            if (rr->st_ <= r->st_ && r->en_ <= rr->en_)
            {
                s.insert(rr);
            }
            else
            {
                break;
            }
            ++it;
        }
    }

  void merge(const ChildRange * r, std::set<ChildRange *> & s, uint64_t & st, uint64_t & en)
  {
    st = r->st_;
    en = r->en_;

    /*
      compute the min st value and max en value for the range set
     */
    for (std::set<ChildRange *>::iterator it = s.begin(); it != s.end(); ++it) {
      ChildRange * rr = *it;
      st_map.erase(rr->st_);
      en_map.erase(rr->en_);
      st = std::min(st, rr->st_);
      en = std::max(en, rr->en_);
      ranges.erase(rr);
    }
  }

  void add(const ChildRange * r)
  {
    std::map<uint64_t, ChildRange *>::iterator it;
    std::set<ChildRange *> s;
    uint64_t st = 0;
    uint64_t en = 0;
    ChildRange * new_r = NULL;
    ParentRange * new_pr = NULL;

    intersect(r, s);

    included(r, s);

    merge(r, s, st, en);

    /* multiple subintervals... create a parent range */
    if (!s.empty())
    {
        new_pr = new ParentRange(st, en);
        new_pr->insertSingleChild(const_cast<ChildRange *>(r));
        new_pr->insertSingleCB(r->cb_);
        new_pr->type_ = r->type_;
        new_pr->handle_ = r->handle_;
        new_r = dynamic_cast<ChildRange *>(new_pr);
    }
    /* a single interval... */
    else
    {
        new_r = const_cast<ChildRange *>(r); /* assign the child to the local child */
    }

    /* iterate of the set of ranges */
    for (std::set<ChildRange *>::iterator it = s.begin(); it != s.end(); ++it)
    {
        ChildRange * rr = *it;

        /* if this is a parent, extract the children from the parent and add them to the new parent */
        if (rr->hasSubIntervals())
        {
            ParentRange * pr = dynamic_cast<ParentRange *>(rr);

            /* copy the child ranges */
            new_pr->insertMultipleChildren(pr->child_ranges_);
            pr->child_ranges_.clear();

            /* copy the child CBs */
            new_pr->insertMultipleCBs(pr->child_cbs_);
            pr->child_cbs_.clear();

            delete pr; /* consolidated parent pr with parent new_pr... delete pr */
        }
        /* if this is a child, add it to the new parent */
        else
        {
            /* add the child range */
            new_pr->insertSingleChild(rr);

            /* add the child cb */
            new_pr->insertSingleCB(rr->cb_);
        }
    }

    ranges.insert(new_r);
    st_map[st] = new_r;
    en_map[en] = new_r;
  }

private:
    std::set<ChildRange *> ranges;
    std::map<uint64_t, ChildRange *> st_map;
    std::map<uint64_t, ChildRange *> en_map;
};

//===========================================================================
}

#endif
