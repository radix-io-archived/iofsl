#include "iofwd/IntervalTreeRangeSet.hh"

/* make this range set configurable */
GENERIC_FACTORY_CLIENT(std::string,
      iofwd::BaseRangeSet,
      iofwd::IntervalTreeRangeSet,
      "intervaltree",
      intervaltree);

namespace iofwd
{
}
