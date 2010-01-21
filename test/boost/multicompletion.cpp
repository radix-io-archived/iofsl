#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "ThreadSafety.hh"

#include "iofwdevent/DummyResource.hh"
#include "iofwdevent/MultiCompletion.hh"

using namespace boost;

const size_t THREADCOUNT = 32;


//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( multicompletion )

struct F {
   iofwdevent::DummyResource dummy_;
};

//____________________________________________________________________________//

// this test case will use struct F as fixture
BOOST_FIXTURE_TEST_CASE( singlethreaded, F )
{
}

BOOST_AUTO_TEST_SUITE_END()


// EOF
