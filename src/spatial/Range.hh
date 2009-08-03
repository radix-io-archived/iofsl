#ifndef SPATIAL_RANGE_HH
#define SPATIAL_RANGE_HH

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

namespace spatial
{
//===========================================================================

class Range
{
public:
  Range() : buf(NULL), st(0), en(0) {}
  Range(uint64_t st, uint64_t en) : st(st), en(en) {}

  enum RangeType {
    RANGE_WRITE = 0,
    RANGE_READ
  };

  RangeType type;
  zoidfs::zoidfs_handle_t * handle;
  char * buf;
  uint64_t st;
  uint64_t en;

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
}

#endif
