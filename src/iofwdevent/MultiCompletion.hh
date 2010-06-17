#ifndef IOFWDEVENT_MULTICOMPLETION_HH
#define IOFWDEVENT_MULTICOMPLETION_HH

#include <boost/thread.hpp>
#include <boost/array.hpp>

#include "iofwdutil/assert.hh"
#include "iofwdevent/CBType.hh"

namespace iofwdevent
{
   //========================================================================

   /**
    * Handles multiple blocking events.
    * The template parameter specifies the maximum number of
    * concurrent callbacks.
    *
    * This class calls a callback after a user-specified number of its slots
    * have completed. It can be used to wait for a the completion of any, all
    * or some of its slots.
    *
    * Each slot consumes less than 64 bytes.
    */
   template <size_t COUNT>
   class MultiCompletion
   {
   protected:
      /**
       * Slot status: either a completion result (such as COMPLETED,
       * CANCELLED,...) or WAITING indicating the slot is armed but waiting
       * for a callback. FREE indicates the slot is unused.
       */
      enum { WAITING = iofwdevent::LAST, FREE };

   public:
      MultiCompletion ();

      ~MultiCompletion ();

      /**
       * When used in a callback context, adds the newly generated CB to its
       * watchlist.
       */
      operator CBType ();


      /**
       * Returns the slot number that completed 
       * or -1 if no slot completed.
       * If a slot completed, status is set to the completion status.
       */
      int testAny (int & status);

      /**
       * Returns number of completed slots,
       * and copies slot numbers to slots array and statuses to status array.
       * No more than max slots will be completed.
       *
       * Slots or status can be 0, in which case the data will not be
       * returned.
       */
      int testSome (size_t slots[], int status[], size_t max);

      /**
       * Calls the callback when at least count slots completed.
       * The callback can use testAny or testSome to determine what slots
       * completed and their status.
       *
       * If count == 0, it will default to the number of waiting operations.
       */
      void wait (const CBType & cb, size_t count = 0);

      /**
       * Cancel pending wait.
       */
      bool cancel ();

   protected:

      /// Returns callback for completing the specified slot.
      inline CBType getSlotCB (size_t pos);


      template <size_t P>
      void completeSlotT (int status)
      { completeSlot (P, status); }

      inline void completeSlot (size_t slot, int status);

      // Remove one completed slot from the system
      bool removeCompleted (size_t & pos, int & status);

   protected:

      // Note: in the future, the slot will also need to store 
      // exceptions...
      struct Slot
      {
         int next_;
         int status_;
      };

   protected:
      boost::array<Slot,COUNT> slots_;

      size_t slots_waiting_;
      size_t slots_completed_;

      int first_completed_;
      int first_free_;

      boost::mutex lock_;

      size_t waiting_for_;

      CBType cb_;
   };

   //========================================================================
   //========================================================================
   //========================================================================

   template <size_t C>
   MultiCompletion<C>::operator CBType ()
   {
      boost::mutex::scoped_lock l(lock_);

      ALWAYS_ASSERT(first_free_ >= 0 && first_free_ < static_cast<int>(C));

      Slot & s = slots_[first_free_];
      const size_t thisslot = first_free_;

      first_free_ = s.next_;
      s.next_ = -1;
      ALWAYS_ASSERT(FREE == s.status_);
      s.status_ = WAITING;

      ++slots_waiting_;

      return getSlotCB (thisslot);
   }

 
   template <size_t C>
   CBType MultiCompletion<C>::getSlotCB (size_t pos)
   {
      ALWAYS_ASSERT(pos < C);

      /* The reason for the switch is that CBType needs to dynamically
       * allocate memory if it binds more than 2 parameters. (member func and
       * this pointer). This makes it slow to copy and pass around.
       * By instantiating completeSlotT for fixed slot numbers, we can avoid 
       * this binding and the associated overhead.
       */
      switch (pos)
      {
         case 0: return boost::bind(&MultiCompletion::template completeSlotT<0>,
                       boost::ref(*this), _1);
         case 1: return boost::bind(&MultiCompletion::template completeSlotT<1>,
                       boost::ref(*this), _1);
         case 2: return boost::bind(&MultiCompletion::template completeSlotT<2>,
                       boost::ref(*this), _1);
         case 3: return boost::bind(&MultiCompletion::template completeSlotT<3>,
                       boost::ref(*this), _1);
         case 4: return boost::bind(&MultiCompletion::template completeSlotT<4>,
                       boost::ref(*this), _1);
         case 5: return boost::bind(&MultiCompletion::template completeSlotT<5>,
                       boost::ref(*this), _1);
         case 6: return boost::bind(&MultiCompletion::template completeSlotT<6>,
                       boost::ref(*this), _1);
         default:
                 return boost::bind(&MultiCompletion::completeSlot, boost::ref(*this),
                       pos, _1);
      }
   }


   template <size_t C>
   MultiCompletion<C>::MultiCompletion ()
      : slots_waiting_(0), slots_completed_(0), first_completed_(-1),
      first_free_(0), waiting_for_(0)
   {
      for (size_t i=0; i<C; ++i)
      {
         slots_[i].next_ = i+1;
         slots_[i].status_ = FREE;
      }
      slots_[C-1].next_ = -1;
   }

   template <size_t C>
   MultiCompletion<C>::~MultiCompletion()
   {
      ALWAYS_ASSERT(0==slots_waiting_);
   }

   template <size_t C>
   void MultiCompletion<C>::completeSlot (size_t slot, int status)
   {
      {
         boost::mutex::scoped_lock l (lock_);
         Slot & s = slots_[slot];

         ALWAYS_ASSERT(WAITING == s.status_);
         ALWAYS_ASSERT(slots_waiting_);

         ++slots_completed_;
         --slots_waiting_;

         s.status_ = status;
         s.next_ = first_completed_;
         first_completed_ = slot;

         if  (waiting_for_ && waiting_for_ <= slots_completed_)
         {
            // make sure we don't call the cb twice
            ASSERT(!cb_.empty());
            waiting_for_ = 0;
         }
         else
         {
            return;
         }
      }

      cb_ (COMPLETED);
      cb_.clear ();
   }
      
   // Must be called with lock held
   template <size_t C>
   bool MultiCompletion<C>::removeCompleted (size_t & pos, int & status)
   {
     if (!slots_completed_)
         return false;

      ASSERT(first_completed_ >= 0);

      const size_t thisslot = first_completed_;

      // Check it has a valid CB status (i.e. not WAITING/FREE)
      Slot & s = slots_[thisslot];
      ASSERT(s.status_ < iofwdevent::LAST);
      status = s.status_;

      // Remove from completed chain
      first_completed_ = s.next_;

      // Add to free chain
      s.next_ = first_free_;
      first_free_ = thisslot;
      s.status_ = FREE;

      --slots_completed_;

      pos = thisslot;
      return true;
   }

   template <size_t C>
   int MultiCompletion<C>::testAny (int & status)
   {
      boost::mutex::scoped_lock l(lock_);
      size_t pos;
      
      if (!removeCompleted(pos, status))
         return -1;

      return pos;
    }

   template <size_t C>
   int MultiCompletion<C>::testSome (size_t * slots, int * status, size_t max)
   {
      boost::mutex::scoped_lock l(lock_);

      size_t dummy1;
      int    dummy2;

      size_t count = 0;

      if (!max)
         max = slots_completed_;

      while (count < max)
      {
         if (!removeCompleted ((slots ? slots[count] : dummy1),
                  (status ? status[count] : dummy2)))
            return count;

         ++count;
      }
      return count;
   }
   
   template <size_t C>
   bool MultiCompletion<C>::cancel ()
   {
      boost::mutex::scoped_lock l(lock_);

      if (!waiting_for_)
         return false;
      waiting_for_ = 0;
      cb_.clear ();
      return true;
   }

   template <size_t C>
   void MultiCompletion<C>::wait (const CBType & cb, size_t count)
   {
      boost::mutex::scoped_lock l(lock_);

      if (!count)
         count = slots_waiting_ + slots_completed_;
      
      // make sure we aren't already waiting
      ALWAYS_ASSERT(!waiting_for_);
      ALWAYS_ASSERT(cb_.empty());

      // If we can complete right away, do it.
      // Unlock before calling callback
      if (count == slots_completed_)
      {
         l.unlock ();
         cb (COMPLETED);
         return;
      }

      waiting_for_ = count;
      cb_ = cb;
   }


   //========================================================================
}

#endif
