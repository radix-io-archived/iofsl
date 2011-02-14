#ifndef TEST_TRACKLIVE_HH
#define TEST_TRACKLIVE_HH

#include <boost/thread.hpp>
#include "iofwdutil/assert.hh"
#include "iofwdutil/atomics.hh"

//===========================================================================
//===== Helper class to track number of live State machines =================
//===========================================================================

/**
 * All state machines deriving from this class will be tracked.
 * The waitAll() method will block until the last tracked client
 * exits.
 */
class TrackLive
{
public:
   TrackLive ()
   {
      ++counter_;
   }

   ~TrackLive ()
   {
      ALWAYS_ASSERT(counter_ > 0);
      if (--counter_)
         return;

      boost::mutex::scoped_lock l (lock_);
      cond_.notify_all ();
   }

   static void waitAll ()
   {
      boost::mutex::scoped_lock l (lock_);
      while (counter_)
      {
         cond_.wait (l);
      }
   }

protected:
   static boost::mutex                 lock_;
   static boost::condition_variable    cond_;

   static iofwdutil::fast_atomic<int>  counter_;
};

#endif
