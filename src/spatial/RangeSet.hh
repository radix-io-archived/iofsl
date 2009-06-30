#ifndef SPATIAL_RANGESET_HH
#define SPATIAL_RANGESET_HH

#include <set>
#include <map>

#include "Range.hh"

namespace spatial
{
//===========================================================================

class RangeSet
{
public:
  unsigned int size() const
  {
    return ranges.size();
  }

  bool empty() const
  {
    return ranges.empty();
  }

  void pop_front(Range& r)
  {
    r = *ranges.begin();
    st_map.erase(r.st);
    en_map.erase(r.en);
    ranges.erase(ranges.begin());
  }

  void add(const Range& r)
  {
    std::map<uint64_t, Range>::iterator it;
    std::set<Range> s;

    // intersect
    it = st_map.lower_bound(r.st);
    while (it != st_map.end()) {
      const Range& rr = it->second;
      if (r.st <= rr.st && rr.st <= r.en) s.insert(rr);
      else break;
      ++it;
    }
    it = en_map.lower_bound(r.st);
    while (it != en_map.end()) {
      const Range& rr = it->second;
      if (r.st <= rr.en && rr.en <= r.en) s.insert(rr);
      else break;
      ++it;
    }

    // included
    it = en_map.lower_bound(r.en);
    while (it != en_map.end()) {
      const Range& rr = it->second;
      if (rr.st <= r.st && r.en << rr.en) s.insert(rr);
      else break;
      ++it;
    }

    // merge
    uint64_t st = r.st;
    uint64_t en = r.en;
    for (std::set<Range>::iterator it = s.begin(); it != s.end(); ++it) {
      const Range& rr = *it;
      st_map.erase(rr.st);
      en_map.erase(rr.en);
      st = std::min(st, rr.st);
      en = std::max(en, rr.en);
      ranges.erase(rr);
    }

    Range new_r(st, en);
    for (std::set<Range>::iterator it = s.begin(); it != s.end(); ++it) {
      const Range& rr = *it;
      if (!rr.child_ranges.empty()) {
        new_r.child_ranges.insert(new_r.child_ranges.begin(),
                                  rr.child_ranges.begin(), rr.child_ranges.end());
      } else {
        new_r.child_ranges.insert(new_r.child_ranges.begin(),
                                  rr);
      }
    }
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
