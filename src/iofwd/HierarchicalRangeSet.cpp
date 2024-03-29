#include "iofwd/HierarchicalRangeSet.hh"
#include "iofwdutil/LinkHelper.hh"

/* make this range set configurable */
GENERIC_FACTORY_CLIENT(std::string,
      iofwd::BaseRangeSet,
      iofwd::HierarchicalRangeSet,
      "hierarchical",
      hierarchical);

namespace iofwd
{
//===========================================================================

//===========================================================================
}
