#ifndef IOFWDEVENT_TIMERRESOURCE_HH
#define IOFWDEVENT_TIMERRESOURCE_HH

#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <queue>
#include <csignal>
#include <boost/thread/thread_time.hpp>
#include <set>
#include <map>

#include "iofwdutil/assert.hh"
#include "iofwdutil/Singleton.hh"
#include "Resource.hh"
#include "ThreadedResource.hh"
#include "iofwdutil/IOFWDLog-fwd.hh"
#include "CBType.hh"

namespace iofwdevent
{
//===========================================================================

class TimerResource : public ThreadedResource,
                      public iofwdutil::Singleton<TimerResource>
{
public:
   TimerResource ();

   virtual ~TimerResource ();

   Handle createTimer (const CBType & cb, unsigned int mstimeout);

   // Cancel timer
   virtual bool cancel (Handle h);

   /// TimerResource needs to do some extra cleanup on stop.
   virtual void stop ();

protected:
   typedef unsigned int seq_type_t;

   struct TimerEntry 
   {
      TimerEntry (seq_type_t seq)
         : sequence_(seq)
      {
      }

      TimerEntry (const CBType & cb, const boost::system_time & a,
            seq_type_t seq)
         : cb_(cb), alarm_(a), sequence_(seq) { }

      CBType cb_;
      boost::system_time alarm_;
      seq_type_t sequence_;
   };

   // Compare the sequence ID's of 
   /*struct IDComp
   {
      bool operator () (const TimerEntry * v1, const TimerEntry * v2)
      { return v1->sequence_ > v2->sequence_; }
   }; */

   /// Comparison operator for the heap
   struct TimerComp
   {
         bool operator () (const TimerEntry * v1, const TimerEntry * v2)
         { return v1->alarm_ > v2->alarm_; }
   };


   /// Code for the timer resource thread.
   void threadMain ();

protected:

   /// Return alarm of next scheduled timer
   /// Needs to be called with lock held
   const boost::system_time & peekNext () const
   {
      ASSERT(!queue_.empty());
      return (*queue_.begin())->alarm_;
   }

protected:
   typedef std::set<TimerEntry *, TimerComp>  QueueType;
   typedef boost::unordered_map<seq_type_t, TimerEntry *> IDMapType;

   QueueType queue_;
   IDMapType ids_;

   iofwdutil::IOFWDLogSource & log_;

   seq_type_t sequence_;

   // pointer to the timer entry currently being executed by the worker thread
   TimerEntry * executing_;
};


//===========================================================================
}


#endif
