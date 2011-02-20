#define BOOST_TEST_MODULE "Test ZeroCopyTransformOutput"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "iofwdevent/ZeroCopyTransformOutput.hh"
#include "iofwdevent/ZeroCopyMemoryInput.hh"
#include "iofwdutil/transform/GenericTransform.hh"
#include "iofwdevent/CBType.hh"
#include "ThreadSafety.hh"
#include <iostream>
using namespace std;
using namespace iofwdevent;
using namespace iofwdutil;
using namespace boost;

BOOST_AUTO_TEST_SUITE( ZeroCopyTransformOutput );
struct Fixture {
    Fixture()
    {
       BOOST_TEST_MESSAGE_TS( "setup fixture" ); 
    }

    ~Fixture()
    {
       BOOST_TEST_MESSAGE_TS( "teardown fixture" );
    }

protected:
    void * mem_array;
    size_t size;
};

BOOST_FIXTURE_TEST_CASE( testConstructor, Fixture )
{
  /*
  size = 100;
  iofwdevent::ZeroCopyMemoryOutput m (mem_array, size);
  iofwdutil::transform::GenericTransform * n;
  iofwdevent::ZeroCopyTransformOutput t((ZeroCopyOutputStream *)&m, n, (size_t)500);
  */
}
BOOST_AUTO_TEST_SUITE_END()
