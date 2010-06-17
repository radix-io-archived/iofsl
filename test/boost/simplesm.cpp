#define BOOST_TEST_MODULE "SimpleSM"
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include "ThreadSafety.hh"

#include "sm/SMManager.hh"
#include "sm/SimpleSM.hh"
#include "iofwdutil/atomics.hh"

using namespace boost;

//____________________________________________________________________________//

class LiveCounter
{
public:
   LiveCounter ()
   {
      boost::mutex::scoped_lock l(lock_);
      ++count_;
   }


   static void waitDone ()
   {
      boost::mutex::scoped_lock l(lock_);
      while (count_)
      {
         cond_.wait (l);
      }
   }

   ~LiveCounter ()
   {
      boost::mutex::scoped_lock l(lock_);
      if (!--count_)
         cond_.notify_all ();
   }

protected:
   static boost::mutex lock_;
   static boost::condition_variable cond_;
   static int count_;
};

int LiveCounter::count_ (0);
boost::condition_variable LiveCounter::cond_;
boost::mutex              LiveCounter::lock_;

//____________________________________________________________________________//

class SMWrapper
{
public:
   SMWrapper (sm::SMManager & m)
      : m_(m)
   {
      m_.startThreads ();
   }

   ~SMWrapper ()
   {
      m_.stopThreads ();
   }

protected:
   sm::SMManager & m_;
};


//____________________________________________________________________________//

class StartStop : public sm::SimpleSM<StartStop>, public LiveCounter
{
protected:
   typedef StartStop ME;

public:

   StartStop (sm::SMManager & smm, bool callnested = false)
      : sm::SimpleSM<StartStop>(smm), callnested_(callnested)
   {
      BOOST_TEST_MESSAGE_TS(format("In constructor: calling nested=%u") % callnested_);
   }

   ~StartStop ()
   {
      BOOST_TEST_MESSAGE_TS("In destructor");
   }

   void init (int )
   {
      BOOST_TEST_MESSAGE_TS("In Init");
   }

protected:
   bool callnested_;
};

//____________________________________________________________________________//

BOOST_AUTO_TEST_SUITE( SimpleSM )

struct F {
   sm::SMManager smm_;

};

//____________________________________________________________________________//


// this test case will use struct F as fixture
BOOST_FIXTURE_TEST_CASE( basic, F )
{
   SMWrapper wrapper (smm_);

   smm_.schedule (new StartStop (smm_));

   BOOST_TEST_MESSAGE_TS("Waiting until all SM's are done");
   LiveCounter::waitDone ();
}

BOOST_AUTO_TEST_SUITE_END()


// EOF
