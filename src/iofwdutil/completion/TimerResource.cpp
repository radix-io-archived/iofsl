#include <limits>
#include <time.h>
#include "TimerResource.hh"

using namespace boost::posix_time; 

namespace iofwdutil
{
   namespace completion
   {

static void safe_sleep (const time_duration & d)
{
   int ret = usleep (static_cast<useconds_t>(d.total_microseconds ())); 
   
   // Do not handle interruption / other errors
   ALWAYS_ASSERT(ret==0); 
}

//===========================================================================

      TimerResource::TimerResource ()
         : comp_ (*this), heap_(comp_)
      {
      }

      void TimerResource::createTimer (TimerCompletionID * id, unsigned int mstimeout)
      {
         boost::mutex::scoped_lock l (lock_); 

         id->alarm_ =  (microsec_clock::universal_time()+
                         millisec (mstimeout)); 
         heap_.push (id); 

         ++freetimers_; 
      }

      bool TimerResource::isActive () const
      {
         // No lock here: assume std::vector<unsigned int>::size is thread
         // safe
         return haveFreeTimers (); 
      }

      /**
       * Needs to be called with lock held
       */
      TimerCompletionID * TimerResource::pop_heap ()
      {
         // pop head
         TimerCompletionID * ele = heap_.top(); 
         heap_.pop (); 

         return ele; 
      }

      
      void TimerResource::waitAny (std::vector<TimerCompletionID *> & completed)
      {
         return testAny (completed, std::numeric_limits<unsigned
               int>::max()); 
      }

      /// Must be called with complock held
      bool TimerResource::removeCompleted (std::vector<TimerCompletionID *> &
            com)
      {
         bool ret = !completed_.empty(); 
         if (!ret)
            return ret; 

         // Remove all the entries from the allocator and put them onto com
         for (size_t i=0; i<completed_.size(); ++i)
         {
            TimerCompletionID * id = completed_[i]; 

            // Reserved entries should not be put on the completed list
            ALWAYS_ASSERT (!id->reserved_);

            com.push_back(id); 
         }
         completed_.clear (); 
         return ret; 
      }

      void TimerResource::testAny (std::vector<TimerCompletionID *> & completed,
            unsigned int maxms)
      {
         bool foundany = false; 
         {
            boost::mutex::scoped_lock lock (complock_); 
            // remove already completed items
            foundany =removeCompleted (completed); 
         }

         // Calculate our timeout
         ptime timeout = microsec_clock::universal_time();

         TimerCompletionID * next; 

         {
            // Try to lock the timer list; If it fails and we have already
            // some entries, prefer returning
            boost::mutex::scoped_try_lock trylock (lock_); 
            if (!trylock.owns_lock())
            {
               if (foundany)
                  return;
               trylock.lock (); 
            }

            // Have the lock for sure; Remove all entries that have expired
            while (heap_.size() && peekNext () < timeout)
            {
               TimerCompletionID * id = pop_heap (); 

               // Reserved entries should not be put on the completion list
               if (!id->reserved_)
               {
                  foundany = true; 
                  completed.push_back (id); 
                  ASSERT(freetimers_); 
                  --freetimers_; 
               }
            }

            // If we already have something, don't wait
            if (foundany)
               return;
               
               
            // Still haven't found anything. Adjust timeout and see if
            // something unreserved expires before that. If not, give up
            timeout += millisec(maxms); 

            // Remove all unreserved entries from the heap
            do
            {
               if (heap_.empty())
                  return;

               if (!heap_.top()->reserved_)
                  break;
                  
               // Reserved entries can be removed without doing anything
               pop_heap (); 
            } while (true); 

            // There is no unreserved entry in our timeout window. Give up.
            if (peekNext() > timeout)
               return; 

            // Remove next entry from heap and wait for it to complete
            next = pop_heap (); 

            --freetimers_; 
         }

         // Release the lock before waiting
         const time_duration sleeptime = microsec_clock::universal_time () -
           next->alarm_; 
         completed.push_back (next); 

         // Sleep: usleep takes microseconds 
         safe_sleep (sleeptime); 
      }

      void TimerResource::wait (TimerCompletionID * id)
      {
         // Test with unlimited timeout should always succeed
         ALWAYS_ASSERT( test (id, std::numeric_limits<unsigned int>::max())); 
      }

      bool TimerResource::test (TimerCompletionID * id, unsigned int maxms)
      {
         // We find the record; See if it is going to expire in time, if so:
         // reserve it. If it happens to be at the top of the heap, remove it.
         // We need to lock to prevent somebody else from expiring the timer.
         
         // There should be at least one free timer: otherwise it means
         // multiple threads are waiting on this CompletionID, or that there
         // aren't any valid completion IDs

          ptime alarm;
         {
            boost::mutex::scoped_lock lock (lock_); 
         
            ALWAYS_ASSERT (freetimers_ && heap_.size()); 
            //ALWAYS_ASSERT (id->getResourceID() == getResourceID()); 

            alarm = id->alarm_;
            const ptime maxwait = microsec_clock::universal_time() + millisec(maxms); 

            // If this fails, somebody is already testing/waiting on the operation
            ALWAYS_ASSERT(!id->reserved_); 

            // If the entry will not expire within the timeout, give up
            if (alarm > maxwait)
               return false; 

            // Take ownership of the entry and release the lock
            id->reserved_ = true; 

            // If the entry happens to be the first entry to expire, remove it
            if (heap_.top() == id)
               pop_heap (); 
         }

         const time_duration d = microsec_clock::universal_time() - alarm;
         safe_sleep (d); 
         return true; 
      }

//===========================================================================
   }
}


