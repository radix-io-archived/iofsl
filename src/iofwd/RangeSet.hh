#ifndef IOFWD_RANGESET_HH
#define IOFWD_RANGESET_HH

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

//===========================================================================
}

#endif
