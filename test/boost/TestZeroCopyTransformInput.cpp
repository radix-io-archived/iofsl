#define BOOST_TEST_MODULE "Test ZeroCopyTransformInput"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "iofwdevent/ZeroCopyTransformInput.hh"
#include "iofwdevent/ZeroCopyMemoryInput.hh"
#include "iofwdutil/transform/GenericTransform.hh"
#include "iofwdevent/CBType.hh"
#include "ThreadSafety.hh"
#include <iostream>
using namespace std;
using namespace iofwdevent;
using namespace iofwdutil;
using namespace boost;

BOOST_AUTO_TEST_SUITE( ZeroCopyTransformInput );
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
  iofwdevent::ZeroCopyMemoryInput * m;
  iofwdutil::transform::GenericTransform * n;
  iofwdevent::ZeroCopyTransformInput t((ZeroCopyInputStream *)m,n);
}
BOOST_AUTO_TEST_SUITE_END()
