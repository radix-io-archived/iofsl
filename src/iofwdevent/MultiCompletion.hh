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
       * watchlist. Automatically picks a free slot. Will assert when no free
       * slots are available.
       */
      operator CBType ();

      /**
       * Return a callback for the specified slot. Will assert if the slot is
       * already active.
       */
      CBType operator [] (size_t pos);


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
       * returned. (This can be used to clear all completed slots:
       * use testSome (0, 0, 0))
       */
      size_t testSome (size_t slots[], int status[], size_t max);

      /**
       * Calls the callback when at least count slots completed.
       * The callback can use testAny or testSome to determine what slots
       * completed and their status.
       *
       * If count == 0, it will default to the current number of waiting
       * operations. (i.e. the number of armed slots)
       */
      void wait (const CBType & cb, size_t count = 0);

      /**
       * Cancel pending wait.
       */
      bool cancel ();


      /**
       * Return number of free slots in the MultiCompletion instance
       */
      inline size_t avail () const;

      /**
       * Return total number of slots
       */
      inline size_t size() const;

      /**
       * Return number of active slots (completed or waiting to complete)
       */
      inline size_t active () const;

      /**
       * Return number of completed slots
       */
      inline size_t completed () const;

   protected:

      /// Returns callback for completing the specified slot.
      inline CBType getSlotCB (size_t pos);


      template <size_t P>
      void completeSlotT (int status)
      { completeSlot (P, status); }

      inline void completeSlot (size_t slot, int status);

      // Remove one completed slot from the system
      void removeCompleted (size_t pos, int & status);

      // Internal check
      void checkInvariants () const;

   protected:

      // Note: in the future, the slot will also need to store 
      // exceptions...
      struct Slot
      {
         int next_;
         int prev_;
         int status_;
      };

   protected:
      boost::array<Slot,COUNT> slots_;

      size_t slots_waiting_;
      size_t slots_completed_;

      int first_completed_;
      int last_completed_;
      int first_free_;

      mutable boost::mutex lock_;

      size_t waiting_for_;

      CBType cb_;
   };

   //========================================================================
   //========================================================================
   //========================================================================

   template <size_t C>
   CBType MultiCompletion<C>::operator [] (size_t pos)
   {
      boost::mutex::scoped_lock l(lock_);

      ALWAYS_ASSERT(pos >= 0 && pos < C);
      ALWAYS_ASSERT(slots_[pos].status_ == FREE);
      
      Slot & s = slots_[pos];

      // Unhook from free list
      // Forward direction
      if (-1 != s.prev_)
      {
         ASSERT(first_free_ != pos);
         slots_[s.prev_].next_ = s.next_;
      }
      else
      {
         first_free_ = s.next_;
      }

      // Backward direction
      if (-1 != s.next_)
      {
         slots_[s.next_].prev_ = s.prev_;
      }

      s.status_ = WAITING;
      ++slots_waiting_;
      return getSlotCB (pos);
   }

   template <size_t C>
   MultiCompletion<C>::operator CBType ()
   {
      boost::mutex::scoped_lock l(lock_);

      // If this fails, there are no free slots available. Note: a slot only
      // becomes free after it was tested using testAny or testSome
      ALWAYS_ASSERT(first_free_ >= 0 && first_free_ < static_cast<int>(C));

      Slot & s = slots_[first_free_];
      const size_t thisslot = first_free_;

      first_free_ = s.next_;

      if (first_free_ != -1)
         slots_[first_free_].prev_ = -1;

      s.next_ = -1;
      s.prev_ = -1;

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
                 return boost::bind(&MultiCompletion::completeSlot,
                       boost::ref(*this), pos, _1);
      }
   }


   template <size_t C>
   MultiCompletion<C>::MultiCompletion ()
      : slots_waiting_(0), slots_completed_(0), first_completed_(-1),
      last_completed_(-1), first_free_(0), waiting_for_(0)
   {
      for (size_t i=0; i<C; ++i)
      {
         slots_[i].next_ = i+1;
         slots_[i].prev_ = i-1;
         slots_[i].status_ = FREE;
      }
      slots_[C-1].next_ = -1;
   }

   template <size_t C>
   MultiCompletion<C>::~MultiCompletion()
   {
      ALWAYS_ASSERT(0==slots_waiting_
            && "MultiCompletion destroyed with waiting callbacks!");
   }

   template <size_t C>
   void MultiCompletion<C>::completeSlot (size_t slot, int status)
   {
      CBType cb;
      {
         boost::mutex::scoped_lock l (lock_);
      
         checkInvariants ();

         Slot & s = slots_[slot];

         ALWAYS_ASSERT(WAITING == s.status_);
         ALWAYS_ASSERT(slots_waiting_);

         ++slots_completed_;
         --slots_waiting_;

         // Add slot to the end of the completed chain
         s.status_ = status;
         s.next_ = -1;
         s.prev_ = last_completed_;

         if (s.prev_ != -1)
            slots_[s.prev_].next_ = slot;
         
         last_completed_ = slot;

         if (first_completed_ < 0)
         {
            ASSERT(-1 == s.next_);
            ASSERT(-1 == s.prev_);
            first_completed_ = slot;
         }

         if  (waiting_for_)
         {
            // The user already called wait ()
            // Check if the condition is fulfilled
            if (waiting_for_ > slots_completed_)
            {
               // Cannot call user callback yet; wait until another slot
               // completes
               return;
            }
         }
         else
         {
            // User didn't call wait() yet; we cannot do anything here
            return;
         }

         // Make sure nobody else thinks they get to call the callback
         waiting_for_ = 0;
         cb.swap (cb_);
      
         checkInvariants ();
      }

      if (cb)
      {
         cb (COMPLETED);
      }
   }
      
   // Must be called with lock held
   template <size_t C>
   void MultiCompletion<C>::removeCompleted (size_t pos, int & status)
   {

      checkInvariants ();

      ASSERT(slots_completed_);
      ASSERT(first_completed_ >= 0);

      // Check it has a valid CB status (i.e. not WAITING/FREE)
      Slot & s = slots_[pos];
      ASSERT(s.status_ < iofwdevent::LAST);
      status = s.status_;

      // Remove from completed chain

      if (s.next_ != -1)
         slots_[s.next_].prev_ = s.prev_;

      if (s.prev_ != -1)
         slots_[s.prev_].next_ = s.next_;

      // If we remove the first entry, update first pointer
      if (first_completed_ == static_cast<int>(pos))
      {
         first_completed_ = s.next_;
      }

      // If we remove the last completed operation, update the last_completed_
      // list pointer.
      if (last_completed_ == static_cast<int>(pos))
      {
         last_completed_ = s.prev_;
      }

      // Add to the front of the free chain
      s.next_ = first_free_;
      s.prev_ = -1;

      if (first_free_ != -1)
         slots_[first_free_].prev_ = pos;

      first_free_ = pos;
      s.status_ = FREE;

      --slots_completed_;

      ASSERT(slots_completed_ || last_completed_ == -1);
      ASSERT(slots_completed_ || first_completed_ == -1);

      checkInvariants ();
   }

   template <size_t C>
   int MultiCompletion<C>::testAny (int & status)
   {
      boost::mutex::scoped_lock l(lock_);

      if (slots_completed_ == 0)
         return -1;

      ASSERT(first_completed_ != -1);

      const size_t slot = first_completed_;
      removeCompleted(slot, status);
      return slot;
    }

   template <size_t C>
   size_t MultiCompletion<C>::testSome (size_t * slots, int * status, size_t max)
   {
      boost::mutex::scoped_lock l(lock_);

      int    dummy2;

      size_t count = 0;

      if (!max)
         max = slots_completed_;

      const size_t expect = std::min (max, slots_completed_);

      while (count < max)
      {
         const int slot = first_completed_;
         if (slot == -1)
         {
            ASSERT(slots_completed_ == 0);
            break;
         }

         if (slots)
            slots[count] = slot;

         removeCompleted (slot,
                  (status ? status[count] : dummy2));
         ++count;
      }

      ALWAYS_ASSERT(count == expect);
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
      if (count <= slots_completed_)
      {
         l.unlock ();
         cb (COMPLETED);
         return;
      }

      // We'll have to block
      waiting_for_ = count;
      cb_ = cb;
   }

   template <size_t S>
   inline size_t MultiCompletion<S>::avail () const
   {
      boost::mutex::scoped_lock l(lock_);
      return  S - slots_waiting_ - slots_completed_;
   }

   template <size_t S>
   size_t MultiCompletion<S>::active () const
   {
      boost::mutex::scoped_lock l (lock_);
      return slots_waiting_ + slots_completed_;
   }

   template <size_t S>
   size_t MultiCompletion<S>::completed () const
   {
      boost::mutex::scoped_lock l (lock_);
      return slots_completed_;
   }


   template <size_t S>
   size_t MultiCompletion<S>::size() const
   {
      return S;
   }


   template <size_t S>
   void MultiCompletion<S>::checkInvariants () const
   {
      if (slots_completed_ == 0)
      {
         ALWAYS_ASSERT (last_completed_ == -1);
         ALWAYS_ASSERT (first_completed_ == -1);

      }

      if (slots_completed_ == 1)
      {
         ALWAYS_ASSERT(last_completed_ != -1);
         ALWAYS_ASSERT(last_completed_ == first_completed_);
         ALWAYS_ASSERT(slots_[last_completed_].prev_ == -1);
         ALWAYS_ASSERT(slots_[last_completed_].next_ == -1);
      }

      int pos;
      size_t count;

      const size_t avail = S - slots_completed_ - slots_waiting_;

      pos = first_completed_;
      count =0;

      while (pos >= 0)
      {
         ++count;
         pos = slots_[pos].next_;
      }

      ALWAYS_ASSERT(slots_completed_ == count);

      pos = last_completed_;
      count = 0;
      while (pos >= 0)
      {
         ++count;
         pos = slots_[pos].prev_;
      }

      ALWAYS_ASSERT(slots_completed_ == count);


      pos = first_free_;
      count = 0;
      while (pos >= 0)
      {
         ++count;
         pos = slots_[pos].next_;
      }

      ALWAYS_ASSERT(avail == count);
   }

   //========================================================================
}

#endif
