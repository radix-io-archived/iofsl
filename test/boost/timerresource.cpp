#include <boost/test/unit_test.hpp>
#include <csignal>

#include "iofwdevent/TimerResource.hh"

using namespace boost::unit_test;

BOOST_AUTO_TEST_SUITE( timerresource)

struct SetVal : public iofwdevent::ResourceOp
{
   SetVal (sig_atomic_t * p, sig_atomic_t v)
      : val(p), dest(v)
   {
   }

   void success ()
   { *val = dest; }

   void cancel ()
   { *val = -dest; }

   sig_atomic_t * val;
   sig_atomic_t dest;
};

BOOST_AUTO_TEST_CASE( StartStopTest )
{
   BOOST_TEST_MESSAGE("Running timer test");
   iofwdevent::TimerResource r;

   r.start ();

   sig_atomic_t val = 0;
   SetVal v (&val, 1);
   SetVal v2 (&val, 2);

   r.createTimer (&v, 10);

   while (!val) {}

   BOOST_CHECK_MESSAGE( val == 1, "Timer fired out of order");

   r.createTimer (&v2, 20);

   while (val == 1) {}

   BOOST_CHECK_MESSAGE( val == 2, "Timer fired out of order");

   r.stop ();
   BOOST_TEST_MESSAGE( "Timer test completed..." );
}

BOOST_AUTO_TEST_SUITE_END()

