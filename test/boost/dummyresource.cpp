#include <boost/test/unit_test.hpp>
#include "iofwdevent/DummyResource.hh"
#include "ThreadSafety.hh"

using namespace iofwdevent;


class DummyTest
{
public:
   DummyTest (int expected)
      : expected_(expected), got_ (-1)
   {
   }

   void test () const
   {
      BOOST_CHECK_EQUAL_TS (expected_, got_);
   }

   void operator () (int status)
   {
      // Check we haven't been called yet
      BOOST_CHECK_EQUAL_TS (got_, -1);

      got_ = status;
   }

   ~DummyTest ()
   {
      test ();
   }

protected:
   int expected_;
   int got_;

};


BOOST_AUTO_TEST_SUITE( dummyresource )

struct SCFixture {
   DummyResource res_;
};

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_FIXTURE_TEST_CASE( immediate, SCFixture )
{
    BOOST_TEST_MESSAGE("Testing DummyResource immediate");

    {
       DummyTest t (COMPLETED);
       res_.immediate (boost::ref(t), COMPLETED);
    }

    {
       DummyTest t (CANCELLED);
       res_.immediate (boost::ref(t), CANCELLED);
    }
}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_FIXTURE_TEST_CASE( deferred, SCFixture )
{
    BOOST_TEST_MESSAGE("Testing DummyResource deferred");

    {
       DummyTest t1 (COMPLETED);
       DummyTest t2 (CANCELLED);

       // Boost ref is needed since we don't want to complete a copy
       // of our DummyTest...
       res_.defer (boost::ref(t1), COMPLETED);
       res_.defer (boost::ref(t2), CANCELLED);
       res_.complete ();
    }
}

BOOST_AUTO_TEST_SUITE_END()
