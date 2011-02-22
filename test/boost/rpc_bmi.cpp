#include <boost/test/unit_test.hpp>
#include "ThreadSafety.hh"
#include "iofwdutil/bmi/BMI.hh"
#include "iofwdutil/Singleton.hh"

#include "iofwdevent/BMIResource.hh"

#include "iofwdevent/MultiCompletion.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/ResourceWrapper.hh"

const char * LISTEN = "tcp://127.0.0.1:12346";

using namespace iofwdutil;
using namespace std;
using namespace iofwdevent;

// @TODO: Some of the tests are channel independent; if another channel is
// added, move to default RPC tests.

// ====== Ensure BMI gets initialized once =========

struct BMIInit : public iofwdutil::Singleton<BMIInit>
{
   public:
      BMIInit ()
      {
         iofwdutil::bmi::BMI::setInitServer (LISTEN);
         iofwdutil::bmi::BMI::get();
      }
};

struct Fixture {

/*   Fixture ()
      : init_(BMIInit::instance ()),
        wrapper_ (&BMIResource::instance ())
   {
   }

   BMIInit & init_;
   ResourceWrapper wrapper_; */
};

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_FIXTURE_TEST_CASE (test1, Fixture)
{
   BOOST_TEST_MESSAGE("Testing construct/destruct of BMIRPC");
}




