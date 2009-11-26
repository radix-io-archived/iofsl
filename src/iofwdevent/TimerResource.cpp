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
   : queue_(comp_), log_(iofwdutil::IOFWDLog::getSource ("timer"))
{
}

TimerResource::~TimerResource ()
{
   ALWAYS_ASSERT(!queue_.size());
}

void TimerResource::createTimer (ResourceOp * id, unsigned int mstimeout)
{
   // If we are shutting down, refuse new blocking events
   if (!isRunning() || needShutdown ())
      throw ResourceInactiveException ();

   // Add entry, and see if this alarm occurs before first alarm.
   // If so, wake the alarm thread.
   TimerEntry * e;
   {
      boost::mutex::scoped_lock l (pool_lock_);
      e = mempool_.construct (id,
        boost::get_system_time() + millisec (mstimeout));
   }

   bool wake = false;
   {
      boost::mutex::scoped_lock l (lock_);
      queue_.push (e);

      // If the new alarm is the next one, the worker thread needs to be
      // informed.
      wake = (queue_.top() == e);
   }

   if (wake)
      notify_.notify_one ();
}


void TimerResource::threadMain ()
{
   ZLOG_DEBUG(log_, "Timer worker thread started!");
   boost::mutex::scoped_lock l (lock_);
   while (!needShutdown ())
   {
      while (queue_.size () && peekNext () <= boost::get_system_time ())
      {
         TimerEntry * e = queue_.top ();
         queue_.pop ();
         l.unlock ();
         try
         {
            e->userdata_->success ();
         }
         catch (...)
         {
            ALWAYS_ASSERT(false && "ResourceOp methods should not throw!");
         }

         // Free timer entry; Need to protect with lock: mempool is not 
         // threadsafe.
         {
            boost::mutex::scoped_lock l2(pool_lock_);
            mempool_.destroy (e);
         }

         l.lock ();
      }

      if (needShutdown ())
         break;

      // Race condition here.
      //   if worker thread checks needShutdown () and then is preempted
      //   while the main thread sets shutdown_ and signals the condition
      //   variable while the worker thread is not waiting on it.
      //   It will subsequently go to sleep and remain sleeping.
      //
      //   Solution: wake up every second if there is no work.
      //
      if (queue_.size ())
         notify_.timed_wait (l, peekNext ());
      else
         notify_.timed_wait (l, (boost::get_system_time() +
                  boost::posix_time::seconds(1)));
   }
}

void TimerResource::stop ()
{
   ZLOG_DEBUG(log_, "Stopping timer thread...");

   // Notify thread it needs to finish work
   signalStop ();

   // Make sure to wake it (in case it is waiting for a timer)
   notify_.notify_one ();

   // Wait until it stops
   waitStop ();

   // There should be no contention on this lock now
   boost::unique_lock<boost::mutex> l (lock_, boost::try_to_lock_t ());
   ALWAYS_ASSERT(l.owns_lock());

   // Call cancel on all the remaining requests
   while (queue_.size())
   {
      TimerEntry * e = queue_.top ();
      queue_.pop ();
      try
      {
         e->userdata_->cancel ();
      }
      catch (...)
      {
         ALWAYS_ASSERT(false && "cancel() should not throw");
      }
   }
}

//===========================================================================
}


