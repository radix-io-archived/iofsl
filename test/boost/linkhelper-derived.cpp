#include "linkhelper-derived.hh"

namespace mynamespace
{

   bool LinkHelperDerived::returnTrue () const
   { return true; }

   
   GENERIC_FACTORY_CLIENT(size_t,LinkHelperBase,LinkHelperDerived,66);

}
