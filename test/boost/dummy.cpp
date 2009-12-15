#define BOOST_TEST_MODULE "Dummy"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "ThreadSafety.hh"

using namespace boost;

const size_t THREADCOUNT = 32;


//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( dummy )

struct F {
};

//____________________________________________________________________________//

static void basichelper (boost::barrier & bar)
{
   bar.wait ();

   bar.wait ();
}

// this test case will use struct F as fixture
BOOST_FIXTURE_TEST_CASE( basicdummy, F )
{
   {
      boost::barrier bar (THREADCOUNT);
      boost::thread_group group;

      BOOST_TEST_MESSAGE_TS("Creating threads...");

      for (size_t i=0; i<THREADCOUNT; ++i)
      {
         group.create_thread (boost::bind(basichelper,
                  boost::ref(bar)));
      }

      BOOST_TEST_MESSAGE_TS("Waiting for threads...");

      group.join_all ();

      BOOST_TEST_MESSAGE_TS("All done...");
   }
   BOOST_CHECK_EQUAL_TS(0, 0);
}

BOOST_AUTO_TEST_SUITE_END()


// EOF
