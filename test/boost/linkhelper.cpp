#include <boost/test/unit_test.hpp>
#include "iofwdutil/LinkHelper.hh"
#include "linkhelper-derived.hh"


using namespace std;

BOOST_AUTO_TEST_SUITE (LinkHelper)

struct F
{
};

BOOST_FIXTURE_TEST_CASE ( basic, F )
{
   typedef iofwdutil::Factory<size_t,mynamespace::LinkHelperBase> MyF;
   BOOST_TEST_MESSAGE ("Checking basic functionality");
   

   //mynamespace::do_register();
   GENERIC_FACTORY_CLIENT_REGISTER(size_t,mynamespace::LinkHelperBase,mynamespace::LinkHelperDerived);

   BOOST_CHECK_EQUAL (MyF::instance().size(), 1);
   auto_ptr<mynamespace::LinkHelperBase> ptr (MyF::construct (66)());
   BOOST_CHECK (ptr->returnTrue());
}

BOOST_AUTO_TEST_SUITE_END()

