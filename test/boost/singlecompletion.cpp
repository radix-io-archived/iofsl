#include <unistd.h>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include "iofwdevent/DummyResource.hh"
#include "iofwdevent/SingleCompletion.hh"
#include "ThreadSafety.hh"

using namespace iofwdevent;

const size_t THREADCOUNT = 32;


BOOST_AUTO_TEST_SUITE( singlecompletion )

struct SCFixture {
    SCFixture() : completed_(false), counter_(0) { }
    ~SCFixture()         { }
    
    SingleCompletion comp;

    DummyResource dummy_;

    bool completed_;

    size_t counter_;
    boost::mutex lock_;
    boost::condition_variable cond_;
};

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_FIXTURE_TEST_CASE( singlethreaded_test, SCFixture )
{
    BOOST_TEST_MESSAGE("Testing singlethreaded SingleCompletion::test");

    // arm SingleComp object.


    dummy_.defer (comp);
    BOOST_CHECK_TS (!comp.test ());
    dummy_.complete ();
    BOOST_CHECK_TS (comp.test ());
    
}

//===========================================================================
//===========================================================================
//===========================================================================
    
BOOST_FIXTURE_TEST_CASE( singlethreaded_wait, SCFixture )
{
   BOOST_TEST_MESSAGE("Checking early complete");
    
    // Here the operation completes before we call wait
    dummy_.immediate (comp);

    boost::system_time t1 = boost::get_system_time ();

    // If this doesn't work, it will hang here
    comp.wait ();

    boost::system_time t2 = boost::get_system_time ();
    BOOST_WARN_TS (t2 - t1 < boost::posix_time::milliseconds (1000));
}

//===========================================================================
//===========================================================================
//===========================================================================

static void multithreaded_test_helper1 (SingleCompletion & comp, 
      boost::barrier & bar)
{
   bar.wait ();

   while (!comp.test ())
   {
   }

}

BOOST_FIXTURE_TEST_CASE( multithreaded_test, SCFixture )
{
    BOOST_TEST_MESSAGE("Thread busy polling test...");
       
    
    // NEed to arm the SingleCompletion obj first
    dummy_.defer (comp);

    // We use a barrier to make sure the testing thread is running before
    // we complete the SingleCompletion
    boost::barrier bar (2);

    {
       boost::thread_group group;
       group.create_thread (boost::bind (&multithreaded_test_helper1, 
                boost::ref(comp), boost::ref(bar)));

       bar.wait ();

       dummy_.complete ();

       group.join_all ();
    }
}

//===========================================================================
//===========================================================================
//===========================================================================

static void multithreaded_test_helper2 (SingleCompletion & comp,
      boost::barrier & bar, bool & completed)
{
   bar.wait ();

   comp.wait ();
   BOOST_CHECK_TS (completed);
}

BOOST_FIXTURE_TEST_CASE( multithreaded_test2, SCFixture )
{
   BOOST_TEST_MESSAGE_TS ("Multithreaded, late wait with multiple threads");

   boost::barrier bar (THREADCOUNT + 1);
   {
      boost::thread_group group;
      
      // Need to arm the SingleCompletion object by using it as a callback
      // before calling test/wait on it.
      dummy_.defer (comp);

      for (size_t i=0; i<THREADCOUNT; ++i)
      {
         group.create_thread (boost::bind (&multithreaded_test_helper2,
                  boost::ref(comp), boost::ref(bar),
                  boost::ref(completed_)));
      }


      // We complete the operation before the threads wait
      dummy_.complete ();
      completed_ = true;

      bar.wait ();

      group.join_all ();
   }
}

//===========================================================================
//===========================================================================
//===========================================================================

static void multithreaded_test_helper (SCFixture & f, bool wait,
      boost::barrier & bar)
{
   bar.wait ();

   size_t threadnum;

   {
      boost::mutex::scoped_lock l(f.lock_);
      threadnum = --f.counter_;
       BOOST_TEST_MESSAGE_TS ((boost::format("Helper thread %i waiting: will test SingleCompletion::")
         % f.counter_) << (wait ? "wait()" : "test()"));
      f.cond_.notify_one ();
   }

   if (!wait)
   {
      while (!f.comp.test ())
      {
      }
   }
   else
   {
      f.comp.wait ();
   }

   BOOST_CHECK_TS (f.completed_);
   
   BOOST_TEST_MESSAGE_TS(boost::format("Thread %i finished") % threadnum);

   bar.wait ();
}

static void mt3_helper (SCFixture & f, bool wait, bool mixed)
{
   if (!mixed)
   {
      BOOST_TEST_MESSAGE_TS ("Multithreaded, early " 
         << (wait ? "wait" : "test") << " with multiple threads");
   }
   else
   {
      BOOST_TEST_MESSAGE_TS ("Multithreaded, early mixed with multiple "
            "threads");
   }

   f.counter_ = THREADCOUNT;

   {
      boost::thread_group group;

      boost::barrier bar (THREADCOUNT + 1);
      
      f.dummy_.defer (f.comp);

      for (size_t i=0; i<THREADCOUNT; ++i)
      {
         bool w = (mixed ? i % 2 : wait);

         group.create_thread (boost::bind(&multithreaded_test_helper,
                  boost::ref(f),
                  w, boost::ref(bar)));
      }


      // We complete the operation after threads started
      bar.wait ();

      {
         boost::mutex::scoped_lock l(f.lock_);
         while (f.counter_ != 0)
         {
            f.cond_.wait (l);
         }
      }

      BOOST_TEST_MESSAGE_TS("Master thread completing SingleCompletion");

      f.completed_ = true;
      f.dummy_.complete ();

      BOOST_TEST_MESSAGE_TS("Master thread waiting for helper threads to "
            "join...");

      bar.wait();

      group.join_all ();

   }
   BOOST_TEST_MESSAGE_TS("Test finished");
}

BOOST_FIXTURE_TEST_CASE( multithreaded_test3, SCFixture )
{
   mt3_helper (*this, true, false);
}


BOOST_FIXTURE_TEST_CASE( multithreaded_test4, SCFixture )
{
   mt3_helper (*this, false, false);
}

BOOST_FIXTURE_TEST_CASE( multithreaded_test5, SCFixture )
{
   mt3_helper (*this, false, true);
}


BOOST_AUTO_TEST_SUITE_END()
