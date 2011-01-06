#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/bind.hpp>
#include <limits>
#include <time.h>

#include "TimerResource.hh"
#include "ResourceInactiveException.hh"
#include "iofwdutil/IOFWDLog.hh"

using namespace boost::posix_time;

namespace iofwdevent
{
//==========================================================================

// Consider boost::MultiIndex for this

TimerResource::TimerResource ()
   : queue_(TimerComp ()),
     log_(iofwdutil::IOFWDLog::getSource ("timer")),
     sequence_(0), executing_(0)
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
   ids_.insert (std::make_pair(sequence_, e));

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
         TimerEntry * e = *queue_.begin ();
         queue_.erase (queue_.begin ());

         // Indicate that we picked this entry and that it is too late to
         // cancel it.
         executing_ = e;

         // Remove sequence id from active sequence id list
         IDMapType::iterator iter = ids_.find (e->sequence_);
         ALWAYS_ASSERT ((iter != ids_.end()));
         ids_.erase (iter);

         // Drop lock before calling callback (so the callback can schedule
         // another timer if desired)
         l.unlock ();
         try
         {
            e->cb_ (CBException ());
         }
         catch (...)
         {
            ALWAYS_ASSERT(false && "ResourceOp methods should not throw!");
         }
         l.lock ();

         executing_ = 0;
         delete (e);
      }

      if (needShutdown ())
         break;

      if (queue_.size ())
         cond_.timed_wait (l, peekNext ());
      else
         cond_.wait (l);
   }
}

/**
 *
 * Note that we cannot use the pointer to the TimerEntry struct as an ID,
 * since, if cancel runs after the timer expired it could match another
 * TimerEntry *, especially if a memory pool is in use and the same memory
 * blocks will be reused.
 *
 * Instead we maintain a hashmap of valid sequence ID's, and make sure every
 * timer request gets a unique sequence ID. We need a map instead of a set
 * because cancel needs to be able to call the callback (CANCELLED) *before*
 * returning. Therefore, it has to map a sequence to a valid pointer; With a
 * set, it would have to do a linear search.
 *
 */
bool TimerResource::cancel (Handle h)
{
   TimerEntry * e;

   {
      boost::mutex::scoped_lock l(lock_);
      // h contains the timer sequence id
      seq_type_t seq = reinterpret_cast<intptr_t> (h);

      // check if this entry is currently being executed
      // We need to make sure to block cancel until it completed
      if (executing_ && executing_->sequence_ == seq)
      {
         // The worker thread already picked this to be executed, so normally
         // there is nothing we need to do. However, we need to guarantee that
         // the callback is not going to be called AFTER cancel returns. So we
         // need to wait until executing_ is done.
         ZLOG_INFO (log_, "Cancelling timer entry that is being executed..."
               " busy waiting...");

         l.unlock ();
         do
         {
            // strictly not needed to lock
            boost::mutex::scoped_lock l2(lock_);
            if (executing_ != e || executing_->sequence_ != seq)
               break;
         } while (true);

         // we don't need to free, the worker thread did it already
         return false;
      }

      IDMapType::iterator I = ids_.find (seq);
      if (I == ids_.end ())
      {
         // Timer already completed or is not valid
         return false;
      }

      e = I->second;

      // Remove entry from queue and sequence list
      // This way the worker thread can no longer pick this item.

      // No need to wake up/warn the worker thread, if this was the next timer
      // to be executed than the worker thread might wake up for no reason.
      ids_.erase (I);
      queue_.erase (e);
   }

   // We can drop the lock: the timer is removed so the worker thread cannot
   // call it any longer
   // Call callback and indicate cancel. We drop the main lock in case the
   // callback wants to reschedule.
   e->cb_ (CBException::cancelledOperation (h));

   return true;
}

void TimerResource::stop ()
{

   ZLOG_DEBUG(log_, "Stopping timer thread...");
   ThreadedResource::stop ();

   // There should be no contention on this lock now
   // since our background thread cannot be running any more
   boost::unique_lock<boost::mutex> l (lock_, boost::try_to_lock_t ());
   ALWAYS_ASSERT(l.owns_lock());

   // There should be no remaining requests if everything else did a proper
   // cleanup and cancel.
   ALWAYS_ASSERT(queue_.empty ());

   // Call cancel on all the remaining requests
   QueueType::iterator I = queue_.begin();
   while (I != queue_.end())
   {
      TimerEntry * e = *I;
      try
      {
         e->cb_ (CBException::cancelledOperation (0));
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


