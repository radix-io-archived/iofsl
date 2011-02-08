#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/lognormal_distribution.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/smart_ptr.hpp>

#include "ThreadSafety.hh"

#include "iofwdutil/assert.hh"

#include "iofwdevent/DummyResource.hh"
#include "iofwdevent/MultiCompletion.hh"
#include "iofwdevent/SingleCompletion.hh"

using namespace boost;
using namespace boost::random;
using namespace iofwdevent;
using namespace std;
using namespace boost::posix_time;

const size_t THREADCOUNT = 32;

// @TODO: add tests for exception support
   
//===========================================================================
//===========================================================================
//===========================================================================

mt11213b randgen_;

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_SUITE( multicompletion )

struct F {
   iofwdevent::DummyResource dummy_;
};

//===========================================================================
//===========================================================================

BOOST_FIXTURE_TEST_CASE( singlethreaded, F )
{

}

//===========================================================================
//===========================================================================

struct mt_context
{
   mt_context (size_t s)
      : cb_(s), done_(false)
   {
      cb_.clear();
   }

   void addCB (const CBType & cb)
   {
      boost::mutex::scoped_lock l(lock_);
      cb_.push_back (cb);
      cond_.notify_one();
   }

   void setDone ()
   {
      boost::mutex::scoped_lock l(lock_);
      done_ = true;
      cond_.notify_all ();
   }

   size_t size () const
   {
      boost::mutex::scoped_lock l(lock_);
      return cb_.size();
   }

   bool isDone () const
   {
      boost::mutex::scoped_lock l(lock_);
      return done_;
   }

   bool getCB (CBType & cb)
   {
      boost::mutex::scoped_lock l(lock_);
      while (cb_.empty() && !done_)
      {
         cond_.wait (l);
      }

      if (done_)
         return false;

      // pick random callback
      const unsigned int slot = randgen_() % cb_.size();
      if (slot != (cb_.size() - 1))
      {
         // Move chosen slot to end of array
         cb_.back().swap (cb_[slot]);
      }

      cb.swap (cb_.back());
      cb_.pop_back ();
      return true;
   }

   mutable mutex lock_;
   condition cond_;
   std::vector<unsigned int> free_;
   std::vector<unsigned int> avail_;
   std::vector<CBType> cb_;
   bool done_;

};

static void mt_main (mt_context & ctx)
{
   std::string seed =
      boost::lexical_cast<std::string>(boost::this_thread::get_id());

   mt11213b rand (hash_range (seed.begin(), seed.end()));
   lognormal_distribution<> distrib (10.0, 100.0);

   variate_generator<boost::mt11213b&, boost::lognormal_distribution<> >
      random (rand, distrib);

   while (!ctx.isDone())
   {
      CBType cb;

      // Call callback
      if (!ctx.getCB(cb))
         return;

      const double delay = abs(random());
      this_thread::sleep (microseconds(delay));

      ALWAYS_ASSERT(cb);
      cb (iofwdevent::CBException ());
   }
}

template <size_t C>
void mt_do_test ()
{
   SingleCompletion block;
   MultiCompletion<C> slots;

   const size_t WORKITEMS = 2048;


   mt_context ctx (C);

   thread_group group;
   for (size_t i=0; i<6; ++i)
   {
      group.create_thread (boost::bind(&mt_main, boost::ref(ctx)));
   }

   // Arm all callbacks
   for (size_t i=0; i<slots.size(); ++i)
      ctx.addCB (slots);

   // Wait until all completed
   BOOST_TEST_MESSAGE("Testing if wait (...,0) works");
   slots.wait (block, 0);
   block.wait ();

   BOOST_TEST_MESSAGE("Trying to clear slot status");
   BOOST_CHECK_EQUAL(slots.testSome (0, 0), slots.size());
   BOOST_CHECK_EQUAL (ctx.size(), 0);

   BOOST_TEST_MESSAGE("---> All callbacks completed");

   BOOST_TEST_MESSAGE("Testing partial slot completion");
    
   size_t todo = WORKITEMS;
   size_t tosubmit = todo;

   for (size_t i=0; i<slots.size(); ++i)
   {
      ctx.addCB (slots);
         BOOST_TEST_MESSAGE(str(format("Adding CB (tosubmit=%i)") %
                     tosubmit));
      --tosubmit;
   }

   BOOST_TEST_MESSAGE("Testing testAny with MC::operator CBType()");

   while (todo)
   {
      // Wait for completion of at least one event
      block.reset ();
      slots.wait (block, 1);
      block.wait ();

      const int slot = slots.testAny ();
      BOOST_TEST_MESSAGE(str(format("Slot %i completed (todo=%i)") % slot %
               todo));
      BOOST_CHECK (-1 != slot);

      --todo;

      if (tosubmit)
      {
         // Readd callback
         BOOST_TEST_MESSAGE(str(format("Adding CB (tosubmit=%i)") %
                     tosubmit));
         ctx.addCB (slots);
         --tosubmit;
      }
   }

   // There should be no more waiting slots
   BOOST_CHECK_EQUAL(slots.avail(), slots.size());


   // ================================================================
   // ================================================================
   // ================================================================

   todo = WORKITEMS;
   tosubmit = WORKITEMS;

   BOOST_TEST_MESSAGE("MultiCompletion, multithreaded test pattern 2");

   const unsigned int MAXRET = randgen_ () % 1024;
   boost::scoped_array<unsigned int> slotnums (new unsigned int[MAXRET]);
   boost::scoped_array<int> statuses (new int[MAXRET]);

   while (todo)
   {
      // Check if we have free slots
      const unsigned int cansubmit = std::min<unsigned int> (slots.avail(), tosubmit);
      for (unsigned int i=0; i<cansubmit; ++i)
      {
         ctx.addCB (slots);
      }
      tosubmit -= cansubmit;

      // Now wait for completion of some
      const unsigned int waitcount = (randgen_ () % slots.active()) + 1;
      block.reset ();

      BOOST_TEST_MESSAGE(str(format("Waiting for completion of %i slots") %
               waitcount));
      slots.wait (block, waitcount);
      block.wait ();

      BOOST_CHECK (slots.completed () >= waitcount);

      bool first = true;

      // Get all completed slots
      do
      {
         const unsigned int count = slots.testSome (&slotnums[0],
               MAXRET);
         
         // The first time, we expect at least waitcount slots to be done
         BOOST_CHECK (!first || (count >= std::min(MAXRET, waitcount)));
         first = false;

         if (count <= 0)
            break;

         BOOST_TEST_MESSAGE(str(format("testSome returned %i slots") % count));


         todo -= count;
      } while (true);
   }

   BOOST_TEST_MESSAGE("Pattern 2, done");
   
   // ================================================================
   // ================================================================
   // ================================================================
   
   ctx.setDone ();

   BOOST_TEST_MESSAGE("Waiting for threads to complete");
   group.join_all ();
}

BOOST_FIXTURE_TEST_CASE (multithreaded, F)
{
   mt_do_test<6> ();
   mt_do_test<12> ();
   mt_do_test<100> ();
   mt_do_test<1024> ();
   mt_do_test<2048> ();
}

BOOST_AUTO_TEST_SUITE_END()

//===========================================================================
//===========================================================================
//===========================================================================

