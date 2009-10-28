#include <boost/test/unit_test.hpp>
#include "iofwdevent/SingleCompletion.hh"

//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( singlecompletion )

struct F {
    F() : i( 0 ) { BOOST_TEST_MESSAGE( "setup fixture" ); }
    ~F()         { BOOST_TEST_MESSAGE( "teardown fixture" ); }

    int i;
};

//____________________________________________________________________________//

// this test case will use struct F as fixture
BOOST_FIXTURE_TEST_CASE( my_test1, F )
{
    // you have direct access to non-private members of fixture structure
    BOOST_TEST_MESSAGE("singlecompletion1");
}

//____________________________________________________________________________//

// you could have any number of test cases with the same fixture
BOOST_FIXTURE_TEST_CASE( my_test2, F )
{
    BOOST_TEST_MESSAGE("singlecompletion2");
}

//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE_END()
