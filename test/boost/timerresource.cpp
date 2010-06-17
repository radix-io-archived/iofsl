#include <boost/test/unit_test.hpp>
#include <csignal>

#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/TimerResource.hh"
#include "ThreadSafety.hh"

using namespace boost::unit_test;
using namespace iofwdevent;

BOOST_AUTO_TEST_SUITE( timerresource)

BOOST_AUTO_TEST_CASE( StartStopTest )
{
   BOOST_TEST_MESSAGE_TS("Running timer test");
   iofwdevent::TimerResource r;
   SingleCompletion comp;
   SingleCompletion comp2;

   r.start ();

   r.createTimer ((comp), 100);
   r.createTimer ((comp2), 200);

   comp.wait ();

   comp2.wait ();

   r.stop (); 

   BOOST_TEST_MESSAGE_TS( "Timer test completed..." );
}

BOOST_AUTO_TEST_SUITE_END()

