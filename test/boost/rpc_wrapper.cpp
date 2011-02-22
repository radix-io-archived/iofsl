#include <boost/test/unit_test.hpp>
#include "ThreadSafety.hh"
// #include "rpc/RPCWrapper.hh"
#include "iofwdevent/SingleCompletion.hh"


//using namespace rpc;

struct Fixture {

};

//===========================================================================
//===========================================================================
//===========================================================================


struct testfunc2
{
   typedef int INPUT;
   typedef int OUTPUT;

   static void execute (int in, int & out)
   {
      BOOST_TEST_MESSAGE_TS ("testfunc2 called");
      out = 2*in;
   }
};

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_FIXTURE_TEST_CASE (test_wrapper_1, Fixture)
{
/*   int a;
   iofwdevent::SingleCompletion sync;

   // RPCWrapper provides encode, decode, execute
   RPCClientWrapper<testfunc2> func2;

   RPCLocalExec<int> exec;

   exec (func2, sync) (2, a);
   sync.wait ();

   BOOST_CHECK_EQUAL (a, 4); */
}




