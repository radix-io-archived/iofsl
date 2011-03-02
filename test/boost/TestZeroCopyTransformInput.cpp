#define BOOST_TEST_MODULE "Test ZeroCopyTransformInput"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/random.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include "iofwdevent/ZeroCopyTransformInput.hh"
#include "iofwdevent/ZeroCopyMemoryInput.hh"
#include "iofwdevent/ZeroCopyMemoryOutput.hh"
#include "iofwdutil/transform/GenericTransform.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/IofwdutilLinkHelper.hh"

#include "iofwdevent/CBType.hh"
#include "ThreadSafety.hh"
#include "iofwd_config.h"
#include <iostream>
#include <iterator>
#include <set>
#include <algorithm>
using namespace std;
using namespace iofwdevent;
using namespace iofwdutil;
using namespace boost;
using namespace iofwdutil::transform;
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
    iofwdutil::transform::GenericTransform * encoder;
    iofwdutil::transform::GenericTransform * decoder;
    iofwdevent::ZeroCopyMemoryOutput * testData;
    char * mem_array;
    size_t size;
};

/* Generate Random Data for the test */
char * RandomData (size_t size)
{
  /* Random number generation */
  boost::mt19937 rng;               
  boost::uniform_int<> randomRange(1,256);
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> >
           die(rng, randomRange);   
  /* Allocate new random data section */          
  char * mem = new char[size];
  int rnd;  

  /* fill with random numbers */
  for (size_t x = 0; x < size; x++)
  {
    rnd = die();
    mem[x] = (char)rnd;
  }
  return mem; 
}
static void EncodeComplete (CBException e)
{
  e.check();
}

void TestEncode ( GenericTransform &  e)
{
  size_t memSize = 1000;
  char * mem_array = RandomData(1000);
  iofwdevent::ZeroCopyMemoryInput in((const void *) mem_array, memSize);
  iofwdevent::ZeroCopyTransformInput trans( &in, &e, 10000);
  
  void ** read = new void * [128];
  size_t readSize = 0;
  size_t suggested = 1000;
  CBType cb = boost::bind(&EncodeComplete, _1);
  for (int i = 0; i < 128; i++)
  {
    trans.read((const void **) read, &readSize, (const CBType) cb, 
               suggested);
    cout << "Read Once " << readSize << endl;
    if (readSize == 0)
    {
      break;
    }
    //read++;
  }
  delete mem_array;
}

BOOST_FIXTURE_TEST_CASE( testConstructor, Fixture )
{
  BOOST_TEST_MESSAGE("Registering Transforms...");
  registerIofwdutilFactoryClients ();

  BOOST_TEST_MESSAGE("Registered transforms:");

  /* Determine what encoders/decoders are availible */
  std::set<std::string> encode;
  std::set<std::string> decode;
  std::set<std::string> cantest;
  std::set<std::string> notest;
  
  iofwdutil::transform::GenericTransformEncodeFactory::instance().keys 
      (std::inserter(encode, encode.begin()));
   
  iofwdutil::transform::GenericTransformDecodeFactory::instance().keys 
      (std::inserter(decode, decode.begin()));

  
  BOOST_FOREACH (const std::string & s, encode)
  {
    BOOST_TEST_MESSAGE(format("Encode: %s") % s);
  }

  std::set_intersection (encode.begin(), encode.end(),
       decode.begin(), decode.end(), std::inserter(cantest, cantest.begin()));
  std::set_symmetric_difference (encode.begin(), encode.end(),
       decode.begin(), decode.end(), std::inserter(notest, notest.begin()));

  BOOST_FOREACH (const std::string & s, decode)
  {
    BOOST_TEST_MESSAGE(format("Decode: %s") % s);
  }

  BOOST_FOREACH (const std::string & s, cantest)
  {
    cout << "Can test " << s << endl;
    BOOST_TEST_MESSAGE(format("Will test: %s (have encode and decode)") % s);
  }


  BOOST_FOREACH (const std::string & s, notest)
  {
    BOOST_TEST_MESSAGE(format("Cannot test: %s (encode or decode missing)")
          % s);
  }

  BOOST_FOREACH (const std::string & s, cantest)
  {
     BOOST_TEST_MESSAGE(format("Testing %s") % s);
     cout << "Testing " << s << endl;
     boost::scoped_ptr<GenericTransform> encodetr (
        GenericTransformEncodeFactory::instance().construct(s)());
     boost::scoped_ptr<GenericTransform> decodetr (
        GenericTransformDecodeFactory::instance().construct(s)());

    TestEncode (* encodetr);
 }
  /*
  iofwdevent::ZeroCopyMemoryInput * m;
  iofwdutil::transform::GenericTransform * n;
  iofwdevent::ZeroCopyTransformInput t((ZeroCopyInputStream *)m,n);
  */
}
BOOST_AUTO_TEST_SUITE_END()
