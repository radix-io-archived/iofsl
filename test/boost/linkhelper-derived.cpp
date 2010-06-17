#include "linkhelper-derived.hh"
   
GENERIC_FACTORY_CLIENT(size_t,mynamespace::LinkHelperBase,
                              mynamespace::LinkHelperDerived,66,linkkey);

namespace mynamespace
{

   bool LinkHelperDerived::returnTrue () const
   { return true; }

   

}
