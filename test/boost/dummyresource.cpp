#include <boost/test/unit_test.hpp>
#include "iofwdevent/DummyResource.hh"
#include "ThreadSafety.hh"

using namespace iofwdevent;


class DummyTest
{
public:
   DummyTest (bool success)
      : expected_(success), success_ (false),
        called_(false)
   {
   }

   void test () const
   {
      BOOST_CHECK_EQUAL_TS (expected_, success_);
   }

   void operator () (iofwdevent::CBException e)
   {
      // Check we haven't been called yet
      BOOST_CHECK_TS (!called_);

      called_ = true;

      if (e.hasException ())
      {
         if (e.isCancelled ())
         {
            success_ = false;
         }
         else
         {
            // unknown exception??
            e.check ();
         }
      }
      else
         success_ = true;
   }

   ~DummyTest ()
   {
      test ();
   }

protected:
   bool expected_;
   bool success_;
   bool called_;
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
       DummyTest t (true);
       res_.immediate (boost::ref(t));
    }

    {
       DummyTest t (false);
       res_.immediate (boost::ref(t),
             iofwdevent::CBException::cancelledOperation (0));
    }
}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_FIXTURE_TEST_CASE( deferred, SCFixture )
{
    BOOST_TEST_MESSAGE("Testing DummyResource deferred");

    {
       DummyTest t1 (true);
       DummyTest t2 (false);

       // Boost ref is needed since we don't want to complete a copy
       // of our DummyTest...
       res_.defer (boost::ref(t1));
       res_.defer (boost::ref(t2),
             iofwdevent::CBException::cancelledOperation (0));
       res_.complete ();
    }
}

BOOST_AUTO_TEST_SUITE_END()
