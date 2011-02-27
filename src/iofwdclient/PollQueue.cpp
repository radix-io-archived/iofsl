#include "PollQueue.hh"

#include "iofwdutil/assert.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>

using namespace boost::posix_time;

namespace iofwdclient
{
   //========================================================================

   PollQueue::PollQueue ()
      : nextid_ (0),
        polling_ (false)
   {
   }

   unsigned int PollQueue::poll_unprotected (unsigned int minms,
         unsigned int maxms)
   {
      // Needs work
      ALWAYS_ASSERT(pollqueue_.size () <= 1);

      unsigned int done = 0;
      BOOST_FOREACH (const PollEntry & e, pollqueue_)
      {
         done += e.func (minms, maxms);
         if (done)
            break;
      }

      return done;
   }

   // Return the number of ms until time 'to'. Returns 0 if 'to' already
   // passed
   static inline unsigned int ms_until (const ptime & now, const ptime &
         to)
   {
      return (now < to ? (to-now).total_milliseconds () : 0);
   }

   //
   // Might do something smarter here, such as if another thread comes along
   // have it poll another resource?
   //
   unsigned int PollQueue::poll (unsigned int minwait, unsigned int maxwait)
   {
      ptime mintimeout = boost::get_system_time () + milliseconds(minwait);
      ptime maxtimeout = boost::get_system_time () + milliseconds(maxwait);

      // Try to get the polling right until minwaitms has passed
      {
         boost::mutex::scoped_lock l (poll_lock_);
         while (polling_)
         {
            if (!poll_cond_.timed_wait (l, mintimeout))
            {
               // Timeout; somebody else is still pollling
               return 0;
            }
         }

         polling_ = true;
      }

      // We try to poll for the remainder of the time (until maxtimeout)
      // If we did specify a timeout and it already passed, don't try to poll.
      // Otherwise, if the timeout was zero, try to poll once.
      if ((boost::get_system_time () >= maxtimeout) && maxwait != 0)
         return 0;

      const ptime curtime = boost::get_system_time ();

      unsigned int ret = poll_unprotected (ms_until (curtime, mintimeout),
            ms_until (curtime, maxtimeout));

      // Release polling token and wake waiters
      boost::mutex::scoped_lock l (poll_lock_);
      polling_ = false;
      poll_cond_.notify_one ();
      return ret;
   }

   PollQueue::PollID PollQueue::registerFunc (const PollFunc & f)
   {
      boost::mutex::scoped_lock l (lock_);
      PollEntry * e = new PollEntry ();
      e->func = f;

      pollqueue_.push_back (e);
      return e;
   }

   void PollQueue::unregisterFunc (PollQueue::PollID id)
   {
      boost::mutex::scoped_lock l (lock_);

      QueueType::iterator found = pollqueue_.end ();
      for (QueueType::iterator iter = pollqueue_.begin ();
            iter != pollqueue_.end (); ++iter)
      {
         if (&(*iter) == id)
         {
            found = iter;
            break;
         }
      }
      if (found == pollqueue_.end ())
      {
         // todo: better exception
         throw "no such ID!";
      }
      pollqueue_.erase (found);
   }


   //========================================================================
}
