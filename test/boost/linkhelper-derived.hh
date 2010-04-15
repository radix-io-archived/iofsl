#ifndef TEST_BOOST_LINKHELPER_DERIVED_HH
#define TEST_BOOST_LINKHELPER_DERIVED_HH

#include "iofwdutil/LinkHelper.hh"
#include "linkhelper-base.hh"

namespace mynamespace
{

class LinkHelperDerived : public LinkHelperBase
{
   public:
      virtual bool returnTrue () const;

      GENERIC_FACTORY_CLIENT_DECLARE(size_t,LinkHelperBase,LinkHelperDerived);
};

}

#endif
