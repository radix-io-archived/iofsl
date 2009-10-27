#include "iofwdevent/TimerResource.hh"

#define BOOST_TEST_MODULE TimerResource test
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <csignal>

using namespace boost::unit_test;

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
}


