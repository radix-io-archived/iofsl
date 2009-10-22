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
   : queue_(comp_), shutdown_(false), running_(false)
{
}

TimerResource::~TimerResource ()
{
   ALWAYS_ASSERT(!running_);
   ALWAYS_ASSERT(!queue_.size());
}

void TimerResource::createTimer (ResourceOp * id, unsigned int mstimeout)
{
   // If we are shutting down, refuse new blocking events
   if (shutdown_ || !running_)
      throw ResourceInactiveException ();

   // Add entry, and see if this alarm occurs before first alarm.
   // If so, wake the alarm thread.
  
   TimerEntry * e = mempool_.construct (id, 
        boost::get_system_time() + millisec (mstimeout));

   bool wake = false;
   {
      boost::mutex::scoped_lock l (lock_);
      queue_.push (e);

      // If the new alarm is the next one, the worker thread needs to be
      // informed.
      wake = (queue_.top() == e);
   }

   if (wake)
      workercond_.notify_one ();
}


void TimerResource::threadMain ()
{
   boost::unique_lock<boost::mutex> l (lock_);
   while (!shutdown_)
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
         l.lock ();
      }

      if (queue_.size ())
         workercond_.timed_wait (l, peekNext ());
      else
         workercond_.wait (l);
   }
}

void TimerResource::start ()
{
   ALWAYS_ASSERT(!running_);

   // Launch thread
   
   boost::thread newthread (boost::bind (&TimerResource::threadMain,this));
   workerthread_.swap (newthread);
   running_ = true;
}

void TimerResource::stop ()
{
   ALWAYS_ASSERT(running_);
   
   shutdown_ = true;
   workerthread_.join ();

   running_ = false;

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


