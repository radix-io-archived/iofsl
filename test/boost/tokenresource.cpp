#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>
#include <boost/format.hpp>
#include <numeric>

#include "iofwdevent/SingleCompletion.hh"
#include "iofwdevent/TokenResource.hh"
#include "iofwdevent/ResourceWrapper.hh"
#include "ThreadSafety.hh"

using namespace boost::unit_test;
using namespace iofwdevent;


const size_t THREADCOUNT = 4;
const size_t TOKENCOUNT  = 6400;
const size_t ITERATIONS  = 256;

BOOST_AUTO_TEST_SUITE(tokenresource)

class Fixture
{
public:
   Fixture ()
      : wrapper_(&tokens_)
   {
      BOOST_TEST_MESSAGE_TS("TokenResource fixture created...");
   }

   ~Fixture ()
   {
      BOOST_TEST_MESSAGE_TS("TokenResource fixture destroyed...");
   }

   TokenResource tokens_;
   ResourceWrapper wrapper_;
};

static size_t globalcount  = 0;
static boost::mutex globallock;

// ==========================================================================
// ==========================================================================
// ==========================================================================


BOOST_FIXTURE_TEST_CASE(test_count, Fixture)
{
   BOOST_TEST_MESSAGE_TS("Checking if TokenResource limits tokens");

   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), 0);
   tokens_.add_tokens (TOKENCOUNT);
   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), TOKENCOUNT);

   size_t obtained = 0;
   while (tokens_.try_request (1))
   {
      ++obtained;
   }

   BOOST_CHECK_EQUAL_TS (obtained, TOKENCOUNT);
   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), 0);

   BOOST_TEST_MESSAGE_TS("Checking if release works... (singlethreaded)");
   for (size_t i=0; i<obtained; ++i)
   {
      tokens_.release (1);
   }
   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), TOKENCOUNT);

}

// ==========================================================================
// ==========================================================================
// ==========================================================================


/**
 * Multiple threads calling try_request or release, but not mixed.
 */

static void multi_try_request (TokenResource & tokens, size_t & mycount,
      boost::barrier & bar)
{
   mycount = 0;
   size_t localcount = 0;

   bar.wait ();

   while (tokens.try_request (1))
      ++localcount;

   mycount = localcount;

   bar.wait ();
}

static void multi_release (TokenResource & tokens, size_t & mycount,
      boost::barrier & bar)
{
   size_t localcount = mycount;

   bar.wait ();

   for (size_t i=0; i<localcount; ++i)
      tokens.release (1);

   mycount = 0;

   bar.wait ();
}

BOOST_FIXTURE_TEST_CASE(multithreaded_request_release, Fixture)
{
   BOOST_TEST_MESSAGE_TS("Checking if multithreaded try_request works");
   
   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), 0);
   tokens_.add_tokens (TOKENCOUNT);
   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), TOKENCOUNT);

   std::vector<size_t> count (THREADCOUNT, 0);

   boost::barrier bar (THREADCOUNT);

   {
      boost::thread_group group;
      for (size_t i=0; i<THREADCOUNT; ++i)
      {
         group.create_thread (boost::bind (&multi_try_request, boost::ref(tokens_),
                  boost::ref(count[i]), boost::ref(bar)));
      }

      group.join_all ();
   }

   // Sum of used tokens and unused tokens should be equal to TOKENCOUNT
   BOOST_CHECK_EQUAL_TS(0, tokens_.get_tokencount());
   BOOST_CHECK_EQUAL_TS(TOKENCOUNT,
         std::accumulate (count.begin(), count.end(), 0));

   {
      boost::thread_group group;
      for (size_t i=0; i<THREADCOUNT; ++i)
      {
         group.create_thread (boost::bind (&multi_release, boost::ref(tokens_),
                  boost::ref(count[i]), boost::ref(bar)));
      }

      group.join_all ();
   }

   BOOST_CHECK_EQUAL_TS(TOKENCOUNT, tokens_.get_tokencount());
   BOOST_CHECK_EQUAL_TS(0,
         std::accumulate (count.begin(), count.end(), 0));
}

// ==========================================================================
// ==========================================================================
// ==========================================================================

/**
 * Mixes try_request and release.
 */

static void multithreaded_try_request_helper 
                (iofwdevent::TokenResource & tokens, size_t & mycount,
                 boost::barrier & bar)
{
   bar.wait ();

   BOOST_CHECK_EQUAL_TS (tokens.get_tokencount(), TOKENCOUNT);

   bar.wait ();

   for (size_t i=0; i<ITERATIONS; ++i)
   {
      const size_t trycount = (random () % 16);
      if (random () % 2 && (trycount >= mycount))
      {
         if (tokens.try_request (trycount))
         {
            mycount += trycount;
         }
      }
      else
      {
         tokens.release (trycount);
         mycount -= trycount;
      }
   }
}


BOOST_FIXTURE_TEST_CASE(multithreaded_try_request, Fixture)
{
   BOOST_TEST_MESSAGE_TS("Multi-threaded try_request");


   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), 0);
   tokens_.add_tokens (TOKENCOUNT);
   BOOST_CHECK_EQUAL_TS (tokens_.get_tokencount(), TOKENCOUNT);

   std::vector<size_t> count (THREADCOUNT, 0);

   boost::barrier bar(THREADCOUNT);

   {
      boost::thread_group group;
      for (size_t i=0; i<THREADCOUNT; ++i)
      {
         group.create_thread (boost::bind (&multithreaded_try_request_helper, 
                  boost::ref(tokens_),
                  boost::ref(count[i]),
                  boost::ref(bar)));
      }

      group.join_all ();
   }

   // Sum of used tokens and unused tokens should be equal to TOKENCOUNT
   BOOST_CHECK_EQUAL_TS(TOKENCOUNT, 
         std::accumulate (count.begin(), count.end(), 0)
         + tokens_.get_tokencount());

   BOOST_TEST_MESSAGE_TS("Try_request test completed..." );
}

// ==========================================================================
// ==========================================================================
// ==========================================================================

static void multi_request_helper (iofwdevent::TokenResource & tokens,
      boost::barrier & bar)
{
   bar.wait ();
   
   BOOST_CHECK_EQUAL_TS (tokens.get_tokencount(), TOKENCOUNT);
   BOOST_CHECK_EQUAL_TS (globalcount, 0);

   bar.wait ();
      
   SingleCompletion complete;

   for (size_t i=0; i<ITERATIONS; ++i)
   {
      const size_t trycount = (random () % 1024);

      if (i)
      {
         complete.reset ();
      }

      tokens.request (&complete, trycount);
      complete.wait ();

      {
         boost::mutex::scoped_lock l(globallock);
         globalcount += trycount;

         // -- cannot do this check because somebody could acquire more tokens
         // after release was called (see above) but before we adjust
         // globalcount

         // BOOST_CHECK_TS (globalcount <= TOKENCOUNT);
      }
      
      boost::thread::sleep (boost::get_system_time() + boost::posix_time::milliseconds(1));

      tokens.release (trycount);
      
      {
         boost::mutex::scoped_lock l(globallock);

         // -- cannot do this check because somebody could acquire more tokens
         // after release was called (see above) but before we adjust
         // globalcount

         // BOOST_CHECK_TS (globalcount >= trycount);
         globalcount -= trycount;
      }
    }

   bar.wait ();
}


BOOST_AUTO_TEST_CASE(multithreaded_request)
{
   Fixture f;

   BOOST_TEST_MESSAGE_TS("Multi-threaded request");


   globalcount = 0;

   BOOST_CHECK_EQUAL_TS (f.tokens_.get_tokencount(), 0);
   f.tokens_.add_tokens (TOKENCOUNT);
   BOOST_CHECK_EQUAL_TS (f.tokens_.get_tokencount(), TOKENCOUNT);

   boost::barrier bar (THREADCOUNT);

   {
      boost::thread_group group;
      for (size_t i=0; i<THREADCOUNT; ++i)
      {
         group.create_thread (boost::bind (&multi_request_helper, boost::ref(f.tokens_),
                  boost::ref(bar)));
      }
      group.join_all ();
   }

   // All tokens should be released
   BOOST_CHECK_EQUAL_TS(TOKENCOUNT, f.tokens_.get_tokencount());
   BOOST_CHECK_EQUAL_TS(globalcount, 0);

   BOOST_TEST_MESSAGE_TS("request test completed..." );
}

BOOST_AUTO_TEST_SUITE_END()
