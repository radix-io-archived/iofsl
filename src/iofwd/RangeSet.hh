#ifndef IOFWD_RANGESET_HH
#define IOFWD_RANGESET_HH

#include <cstdio>
#include <iostream>
#include <cassert>
#include <set>
#include <map>

#include "Range.hh"

namespace iofwd
{
//===========================================================================


/**
 * TODO: 
 *   convert from dual  map to rtree
 */
class RangeSet
{
public:
  size_t size() const
  {
    return ranges.size();
  }

  bool empty() const
  {
    return ranges.empty();
  }

  void pop_front(Range& r)
  {
    assert(!ranges.empty());
    r = *ranges.begin();
    st_map.erase(r.st);
    en_map.erase(r.en);
    ranges.erase(ranges.begin());
  }

  /* find buffer intersects... no dup ranges are returned */
  void intersect(const Range & r, std::set<Range> & s)
  {
    std::map<uint64_t, Range>::iterator it;

    /*
       The beginning part of rr overlaps the end of r 
       r  [st .... en]
       rr      [st .... en]
    */
    it = st_map.lower_bound(r.st);
    while (it != st_map.end()) {
      const Range& rr = it->second;
      if (r.st <= rr.st && rr.st <= r.en) s.insert(rr);
      else break;
      ++it;
    }

    /*
       The last part of rr overlaps the beginning of r 
       r        [st .... en]
       rr [st .... en]
    */
    it = en_map.lower_bound(r.st);
    while (it != en_map.end()) {
      const Range& rr = it->second;
      if (r.st <= rr.en && rr.en <= r.en) s.insert(rr);
      else break;
      ++it;
    }

  }

  /* identify all of the buffers that are a subset of existing buffers */
  void included(const Range & r, std::set<Range> & s)
  {
    std::map<uint64_t, Range>::iterator it;

    /*
       Buffer rr overlaps all of r 
       r        [st .... en]
       rr [st ................... en]
    */
    it = en_map.lower_bound(r.en);
    while (it != en_map.end()) {
      const Range& rr = it->second;
      if (rr.st <= r.st && r.en <= rr.en) s.insert(rr);
      else break;
      ++it;
    }

  }

  void merge(const Range & r, std::set<Range> & s, uint64_t & st, uint64_t & en)
  {
    st = r.st;
    en = r.en;

    /*
      compute the min st value and max en value for the range set
     */
    for (std::set<Range>::iterator it = s.begin(); it != s.end(); ++it) {
      const Range& rr = *it;
      st_map.erase(rr.st);
      en_map.erase(rr.en);
      st = std::min(st, rr.st);
      en = std::max(en, rr.en);
      ranges.erase(rr);
    }
  }

  void add(const Range& r)
  {
    std::map<uint64_t, Range>::iterator it;
    std::set<Range> s;
    uint64_t st = 0, en = 0;

    intersect(r, s);

    included(r, s);

    merge(r, s, st, en);

    Range new_r(st, en);
    new_r.type = r.type;
    new_r.handle = r.handle;
    if (!s.empty()) {
      new_r.child_ranges.push_back(r);
      new_r.cids.insert(new_r.cids.begin(), r.cids.begin(), r.cids.end());
    } else {
      new_r.buf = r.buf;
      new_r.child_ranges = r.child_ranges;
      new_r.cids = r.cids;
    }
    for (std::set<Range>::iterator it = s.begin(); it != s.end(); ++it) {
      const Range& rr = *it;
      if (!rr.child_ranges.empty()) {
        new_r.child_ranges.insert(new_r.child_ranges.begin(),
                                  rr.child_ranges.begin(), rr.child_ranges.end());
        for (unsigned int i = 0; i < rr.child_ranges.size(); i++)
          new_r.cids.insert(new_r.cids.begin(),
                            rr.child_ranges[i].cids.begin(),
                            rr.child_ranges[i].cids.end());

      } else {
        new_r.child_ranges.insert(new_r.child_ranges.begin(),
                                  rr);
        new_r.cids.insert(new_r.cids.begin(), rr.cids.begin(), rr.cids.end());
      }
    }

    if (!new_r.child_ranges.empty())
      assert(new_r.cids.size() == new_r.child_ranges.size());
    assert(new_r.cids.size() > 0);
    
    ranges.insert(new_r);
    st_map[st] = new_r;
    en_map[en] = new_r;
  }

private:
  std::set<Range> ranges;
  std::map<uint64_t, Range> st_map;
  std::map<uint64_t, Range> en_map;
};

class HierarchicalRangeSet
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

    void pop_front(ChildRange ** r)
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

  void add(ChildRange * r)
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
        //new_pr->child_ranges_.push_back(r);
        new_pr->insertSingleChild(r);
        //new_pr->child_cids_.insert(new_pr->child_cids_.begin(), r->cid_);
        new_pr->insertSingleCid(r->cid_);
        new_pr->type_ = r->type_;
        new_pr->handle_ = r->handle_;
        new_r = dynamic_cast<ChildRange *>(new_pr);
    }
    /* a single interval... */
    else
    {
        new_r = r; /* assign the child to the local child */
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
            //new_pr->child_ranges_.insert(new_pr->child_ranges_.begin(), pr->child_ranges_.begin(), pr->child_ranges_.end());
            new_pr->insertMultipleChildren(pr->child_ranges_);
            pr->child_ranges_.clear();

            /* copy the child cids */
            //new_pr->child_cids_.insert(new_pr->child_cids_.begin(), pr->child_cids_.begin(), pr->child_cids_.end());
            new_pr->insertMultipleCids(pr->child_cids_);
            pr->child_cids_.clear();

            delete pr; /* consolidated parent pr with parent new_pr... delete pr */
        }
        /* if this is a child, add it to the new parent */
        else
        {
            /* add the child range */
            //new_pr->child_ranges_.insert(new_pr->child_ranges_.begin(), rr);
            new_pr->insertSingleChild(rr);

            /* add the child cid */
            new_pr->insertSingleCid(rr->cid_);
        }
    }

    /* final checks... if this is a parent.. check that the cids and subinterval sizes are equal*/
    if (new_r->hasSubIntervals())
    {
        ParentRange * pr = dynamic_cast<ParentRange *>(new_r);
        assert(pr->child_cids_.size() == pr->child_ranges_.size());
        assert(pr->child_cids_.size() > 0);
    }
    /* else, verify that the cid is set */
    else
    {
        assert(new_r->cid_ != NULL);
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
