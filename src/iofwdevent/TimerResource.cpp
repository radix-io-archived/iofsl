#include <limits>
#include <time.h>
#include "TimerResource.hh"
#include "ResourceInactiveException.hh"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/bind.hpp>

using namespace boost::posix_time;

namespace iofwdevent
{
//==========================================================================

TimerResource::TimerResource ()
   : queue_(TimerComp ()), ids_(IDComp()),
     log_(iofwdutil::IOFWDLog::getSource ("timer")),
     sequence_(0)
{
}

TimerResource::~TimerResource ()
{
   ALWAYS_ASSERT(queue_.empty());
   ALWAYS_ASSERT(ids_.empty());
}

TimerResource::Handle TimerResource::createTimer (const CBType & cb, unsigned int mstimeout)
{
   // to shut up valgrind even though there is a race free way (here) 
   // to signal the thread without needing to lock the mutex.
   boost::mutex::scoped_lock l(lock_); 

   // If we are shutting down, refuse new blocking events
   if (!isRunning() || needShutdown ())
      throw ResourceInactiveException ();


   // Add entry, and see if this alarm occurs before first alarm.
   // If so, wake the alarm thread.
   TimerEntry * e = new TimerEntry (cb, boost::get_system_time() + millisec (mstimeout),
         ++sequence_);

   queue_.insert (e);
   ids_.insert (e);

   // If the new alarm is the next one, the worker thread needs to be
   // informed.
   if (*queue_.begin() == e)
      cond_.notify_one ();

   return reinterpret_cast<void*> (intptr_t (e->sequence_));
}

void TimerResource::threadMain ()
{
   ZLOG_DEBUG(log_, "Timer worker thread started!");

   boost::mutex::scoped_lock l (lock_);
   while (!needShutdown ())
   {
      while (!queue_.empty () && peekNext () <= boost::get_system_time ())
      {
         int status = COMPLETED;

         TimerEntry * e = *queue_.begin ();
         queue_.erase (queue_.begin ());

         // If the ID is no longer there it was cancelled
         // If not we remove it
         IDMapType::iterator iter = ids_.find (e);
         if (iter == ids_.end())
         {
            status = CANCELLED;
         }
         else
         {
            ids_.erase (iter);
         }

         l.unlock ();
         try
         {
            e->cb_ (status);
         }
         catch (...)
         {
            ALWAYS_ASSERT(false && "ResourceOp methods should not throw!");
         }

         delete (e);

         l.lock ();
      }

      if (needShutdown ())
         break;

      if (queue_.size ())
         cond_.timed_wait (l, peekNext ());
      else
         cond_.wait (l);
   }
}

bool TimerResource::cancel (Handle h)
{
   boost::mutex::scoped_lock l(lock_);
   TimerEntry e (reinterpret_cast<intptr_t> (h));
   IDMapType::iterator I = ids_.find (&e);
   if (I == ids_.end ())
      return false;
   ids_.erase (I);
   return true;
}

void TimerResource::stop ()
{

   ZLOG_DEBUG(log_, "Stopping timer thread...");
   ThreadedResource::stop ();

   // There should be no contention on this lock now
   boost::unique_lock<boost::mutex> l (lock_, boost::try_to_lock_t ());
   ALWAYS_ASSERT(l.owns_lock());

   // Call cancel on all the remaining requests
   QueueType::iterator I = queue_.begin();
   while (I != queue_.end())
   {
      TimerEntry * e = *I;
      try
      {
         e->cb_ (CANCELLED);
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "cancel() should not throw");
      }
      delete (e);
   }
}

//===========================================================================
}


