#ifndef IOFWDEVENT_TIMERRESOURCE_HH
#define IOFWDEVENT_TIMERRESOURCE_HH

#include <boost/thread.hpp>
#include <queue>
#include <csignal>
#include <boost/pool/object_pool.hpp>
#include "Resource.hh"
#include <boost/thread/thread_time.hpp>
#include "iofwdutil/assert.hh"
#include "ResourceOp.hh"


namespace iofwdevent
{
//===========================================================================

class TimerResource
{
public:
   TimerResource ();

   virtual ~TimerResource ();

   void createTimer (ResourceOp * id, unsigned int mstimeout);


   virtual void stop ();

   virtual void start ();

protected:
   struct TimerEntry
   {
      TimerEntry (ResourceOp * u, const boost::system_time & a)
         : userdata_(u), alarm_(a) { }

      ResourceOp * userdata_;
      boost::system_time alarm_;
   };

   /// Comparison operator for the heap
   class TimerComp
   {
      public:
         bool operator () (const TimerEntry * v1, const TimerEntry * v2)
         {
            return v1->alarm_ > v2->alarm_;
         }
   };


   /// Code for the timer resource thread.
   void threadMain ();

protected:

   /// Return alarm of next scheduled timer
   /// Needs to be called with lock held
   const boost::system_time & peekNext () const
   {
      ASSERT(queue_.size());
      return queue_.top()->alarm_;
   }

protected:
   /// Memory pool for the timer entries
   boost::object_pool<TimerEntry> mempool_;

   /// Comparison functor for queue_;
   TimerComp comp_;

   /// Priority queue to order the alarms
   std::priority_queue<TimerEntry *,
      std::vector<TimerEntry *>, TimerComp> queue_;

   // Protects priority queue
   boost::mutex lock_;

   /// Notify the worker thread it needs to shut down.
   sig_atomic_t  shutdown_;

   bool running_;

   boost::thread workerthread_;

   boost::condition_variable workercond_;
};


//===========================================================================
}


#endif
