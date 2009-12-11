#define BOOST_TEST_MODULE "Atomics library"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include <numeric>
#include <vector>
#include <algorithm>

#include "iofwd_config.h"
#include "iofwdutil/atomics.hh"
#include "ThreadSafety.hh"

using namespace iofwdutil;
using namespace boost;

#define THREADCOUNT 32

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

// --------------------------------------------------------------------------


template <typename T>
void doTestHelper (size_t * remote, T & shared)
{
   size_t local = 0;
   
   for (unsigned int i=0; i<1000; ++i)
   {
      shared += 1;
      local += 1;
   }
   *remote = local;
}
/**
 * Tests atomic +=.
 */
template <typename T>
void doTest ()
{
   T val = 0;
   std::vector<size_t> localval(THREADCOUNT);
   boost::thread_group threads;

   for (int i=0; i<THREADCOUNT; ++i)
   {
      threads.create_thread (boost::bind(&doTestHelper<T>, &localval[i],
               boost::ref(val)));
   }

   threads.join_all ();
   BOOST_CHECK_EQUAL_TS (std::accumulate (localval.begin(), localval.end(), 0),
         static_cast<size_t> (val));
}

BOOST_FIXTURE_TEST_CASE (threaded, F)
{
   if (atomic<int>::USING_LOCKS)
      BOOST_ERROR("atomic<int> is using locks!!");

   doTest<atomic<int> > ();
#ifdef HAVE_ATOMICS
   doTest<fast_atomic<int> > (); 
#endif
}

BOOST_AUTO_TEST_SUITE_END()


// EOF
