#define BOOST_TEST_MODULE "Atomics library"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>

#include "iofwdutil/atomics.hh"

using namespace iofwdutil;
using namespace boost;

//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( atomics )

struct F {
};

//____________________________________________________________________________//

// this test case will use struct F as fixture
BOOST_FIXTURE_TEST_CASE( basic, F )
{
   atomic<int> a;

   atomic<int> a2(1);
   atomic<int> a3;
   atomic<int> a4;

   BOOST_MESSAGE(format("Atomic emulation: %i") % static_cast<int>(atomic<int>::USING_LOCKS));

   BOOST_REQUIRE_EQUAL (1, a2.load ());
   BOOST_REQUIRE_EQUAL (1, a2);

   // Test decr_and_test
   a = 10;
   for (unsigned int i=0; i<9; ++i)
   {
      --a;
   }
   BOOST_MESSAGE(format("Size of atomic int: %i") % sizeof(a));
   BOOST_REQUIRE_EQUAL (a, 1);
   BOOST_REQUIRE(a.decr_and_test());

   a=0; a2=0; a3=0; a4=0;
   for (unsigned int i=0; i<10; ++i)
   {
      ++a; 
      a2++;
      a3+=1;
      a4.incr();
   }
   BOOST_REQUIRE_EQUAL(10, a);
   BOOST_REQUIRE_EQUAL(10, a2);
   BOOST_REQUIRE_EQUAL(10, a3);
   BOOST_REQUIRE_EQUAL(10, a4);

}

BOOST_AUTO_TEST_SUITE_END()


// EOF
