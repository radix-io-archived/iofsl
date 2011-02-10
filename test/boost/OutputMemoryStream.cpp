#define BOOST_TEST_MODULE "Test output memory zero copy implementation"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "iofwdevent/ZeroCopyMemoryOutput.hh"
#include "iofwdevent/CBType.hh"
#include "ThreadSafety.hh"
#include <iostream>
using namespace std;
using namespace iofwdevent;
using namespace boost;
BOOST_AUTO_TEST_SUITE( ZeroCopyMemoryOutput );
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

void testReadCB (CBException e)
{
  return;
}


BOOST_FIXTURE_TEST_CASE( testConstructor, Fixture )
{
  size = 500;
  iofwdevent::ZeroCopyMemoryOutput x((void **)&mem_array, size);
}

BOOST_FIXTURE_TEST_CASE ( testWrite, Fixture)
{
  Handle h;
  CBType cb = &testReadCB;
  size = 501;
  void * readloc;
  size_t readSize; 

  iofwdevent::ZeroCopyMemoryOutput x((void **)&readloc, size);
  h = x.write(((void **)(&mem_array)), &readSize, cb, 501);
  for (int x = 0; x < 500; x++)
  {
    ((char *)mem_array)[x] = (char)(x % 10);
  }
  h = x.flush(cb);

  for (int x = 0; x < 500; x++)
  {
    BOOST_CHECK_EQUAL ( (char)(x % 10), ((char *)readloc)[x]);
  }
}
/*
BOOST_FIXTURE_TEST_CASE (testRewind, Fixture)
{
  Handle h;
  CBType cb = &testReadCB;
  size = 500;
  mem_array = malloc(sizeof(char) * 500);
  void * readloc;
  size_t readSize; 
  for (int x = 0; x < 500; x++)
  {
    ((char *)mem_array)[x] = (char)(x % 10);
  }
  iofwdevent::InputMemoryZeroCopy x((void **)&mem_array, size);
  h = x.read(((const void **)(&readloc)), &readSize, cb, 250);
  h = x.rewindInput(readSize, cb);
  h = x.read(((const void **)(&readloc)), &readSize, cb, 250);
  for (int x = 0; x < readSize; x++)
  {
    BOOST_CHECK_EQUAL ( (char)(x % 10), ((char *)readloc)[x]);
  }
}
*/
BOOST_AUTO_TEST_SUITE_END()
