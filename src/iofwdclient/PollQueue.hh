#ifndef IOFWDCLIENT_POLLQUEUE_HH
#define IOFWDCLIENT_POLLQUEUE_HH

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace iofwdclient
{
   //========================================================================

   class PollQueue
   {
      protected:

         struct PollEntry;

      public:

         // PollFunc polls for at least minwait, at most maxwait and
         // returns number of completed events.
         typedef boost::function<unsigned int (unsigned int, unsigned int)>
                        PollFunc;

         typedef PollEntry * PollID;

         PollQueue ();

         /**
          * Poll for at least minwait ms, at most maxwait ms
          *
          * Returns number of events handled during the wait.
          *
          * This function can be called concurrently from multiple threads.
          */
         unsigned int poll (unsigned int minwait, unsigned int maxwait);

         /**
          * Register given PollFunc for polling.
          * Returns pollid that can be used to just poll the given function
          * and to unregister the function.
          */
         PollID registerFunc (const PollFunc & f);

         /**
          * Remove id from the pollqueue.
          */
         void unregisterFunc (PollID id);

      protected:

         // Needs to be called with lock held;
         // Polls for at most time ms
         unsigned int poll_unprotected (unsigned int mintime,
               unsigned int maxtime);

      protected:
         boost::mutex lock_;

         struct PollEntry
         {
            PollFunc func;
         };

         size_t nextid_;
         typedef boost::ptr_vector<PollEntry> QueueType;
         QueueType pollqueue_;

         boost::mutex              poll_lock_;
         boost::condition_variable poll_cond_;
         bool                      polling_;
   };

   //========================================================================
}

#endif
