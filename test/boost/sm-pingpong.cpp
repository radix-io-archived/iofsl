#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "ThreadSafety.hh"

using namespace boost;

//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( bmi_sm_pingpong )

struct F {
};

//____________________________________________________________________________//
// this test case will use struct F as fixture

BOOST_FIXTURE_TEST_CASE( basicdummy, F )
{
   BOOST_TEST_MESSAGE_TS("Running BMI State machine pingpong example");
   BOOST_TEST_MESSAGE_TS("Waiting for state machines to end...");
   BOOST_TEST_MESSAGE_TS("All done...");
}

BOOST_AUTO_TEST_SUITE_END()


// EOF
