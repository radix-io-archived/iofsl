#ifndef IOFWDEVENT_MULTICOMPLETION_HH
#define IOFWDEVENT_MULTICOMPLETION_HH

#include <boost/thread.hpp>
#include <boost/array.hpp>

#include "iofwdutil/assert.hh"
#include "iofwdevent/CBType.hh"
#include "CompletionException.hh"
#include "CBException.hh"

namespace iofwdevent
{
   //========================================================================

   /**
    * Handles multiple blocking events.
    * The template parameter specifies the maximum number of
    * concurrent callbacks.
    *
    * This class calls a callback after a user-specified number of its slots
    * have completed. It can be used to wait for the completion of any, all
    * or some of its slots.
    *
    * Each slot consumes less than 64 bytes.
    *
    * @TODO: making this a template just to avoid the extra malloc might have
    * been overkill. Consider going to boost intrusive containers (linked
    * list) together with a shared (between all multicompletion instances)
    * memory pool?
    */
   template <unsigned int COUNT>
   class MultiCompletion
   {
   protected:
      /**
       * Slot status:
       * WAITING indicating the slot is armed but waiting for a callback,
       * FREE indicates the slot is unused.
       */
      enum { WAITING, FREE, COMPLETED };

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
      CBType operator [] (unsigned int pos);


      /**
       * Returns the slot number that completed or -1 if no slot completed.
       * If a slot completed with an exception, the exception is rethrown
       * here. The slot returns becomes available again, regardless if an
       * exception occured. In case of an exception, the slot_number tag will
       * be set to the slot number the exception belongs to.
       *
       * NOTE: if it ever is a hassle to deal with this function throwing
       * exceptions, it could be modified to return an exception pointer
       * instead, enabling the caller to deal with the exception on whever it
       * is ready to do so.
       */
      int testAny ();

      /**
       * Returns number of completed slots,
       * and copies slot numbers to slots array.
       * No more than max slots will be completed.
       *
       * Slots can be 0, in which case the data will not be
       * returned. (This can be used to clear all completed slots:
       * use testSome (0, 0))
       *
       * If an exception is present, it will be rethrown;
       * The slot the exception belongs to is indicated in the slot_number
       * tag. The status of the other slots will not chance.
       */
      unsigned int testSome (unsigned int slots[],
            unsigned int max);

      /**
       * Calls the callback when at least count slots completed.
       * The callback can use testAny or testSome to determine which slots
       * completed and their status.
       *
       * If count == 0, it will default to the current number of waiting
       * operations. (i.e. the number of armed slots)
       *
       * If an exception occurs, cb will be called even if less
       * than count slots completed. In this case, the cb will be called with
       * a SlotException exception; The original exception can be retrieved
       * using testAny or testSome.
       *
       * If cancel() is called successfully, cb will be called with an
       * EventCancelledException.
       *
       * Note that cb can be called from within wait if count slots already
       * completed, or if an exception slot is present. In this case, wait ()
       * might not return before cb completes.
       */
      void wait (const CBType & cb, unsigned int count = 0);

      /**
       * Cancel pending wait.
       */
      bool cancel ();


      /**
       * Return number of free slots in the MultiCompletion instance
       */
      inline unsigned int avail () const;

      /**
       * Return total number of slots
       */
      inline unsigned int size() const;

      /**
       * Return number of active slots (completed or waiting to complete)
       */
      inline unsigned int active () const;

      /**
       * Return number of completed slots (including the ones that have a
       * pending exception)
       */
      inline unsigned int completed () const;

      /**
       * Return the number of slots with a pending exception
       */
      inline unsigned int exceptionCount () const;

      /**
       * Return number of a free slot, or -1 if no slots are free.
       */
      inline int nextFree () const;

      /**
       * Returns true if the specified slot is free.
       */
      inline bool isFree (unsigned int slot) const;

   protected:

      /// Returns callback for completing the specified slot.
      inline CBType getSlotCB (unsigned int pos);


      template <unsigned int P>
      void completeSlotT (CBException e)
      { completeSlot (P, e); }

      inline void completeSlot (unsigned int slot, CBException e);

      // Remove one completed slot from the system
      void removeCompleted (unsigned int pos, CBException & status);

      // Internal check
      void checkInvariants () const;

   protected:

      struct Slot
      {
         int next_;
         int prev_;
         int status_;
         CBException exception_;
      };

   protected:
      boost::array<Slot,COUNT> slots_;

      unsigned int slots_waiting_;
      unsigned int slots_completed_;

      int first_completed_;
      int last_completed_;
      int first_free_;

      // The number of slots with a pending exception
      unsigned int slots_exception_;

      mutable boost::mutex lock_;

      unsigned int waiting_for_;

      CBType cb_;
   };

   //========================================================================
   //========================================================================
   //========================================================================

   template <unsigned int C>
   CBType MultiCompletion<C>::operator [] (unsigned int pos)
   {
      boost::mutex::scoped_lock l(lock_);

      ALWAYS_ASSERT(pos < C);
      ALWAYS_ASSERT(slots_[pos].status_ == FREE);
      
      Slot & s = slots_[pos];

      // Unhook from free list
      // Forward direction
      if (-1 != s.prev_)
      {
         ASSERT(first_free_ != static_cast<int>(pos));
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
      ASSERT(!s.exception_.hasException ());
      ++slots_waiting_;
      return getSlotCB (pos);
   }

   template <unsigned int C>
   MultiCompletion<C>::operator CBType ()
   {
      boost::mutex::scoped_lock l(lock_);

      // If this fails, there are no free slots available. Note: a slot only
      // becomes free after it was tested using testAny or testSome
      ALWAYS_ASSERT(first_free_ >= 0 && first_free_ < static_cast<int>(C));

      Slot & s = slots_[first_free_];
      const unsigned int thisslot = first_free_;

      first_free_ = s.next_;

      if (first_free_ != -1)
         slots_[first_free_].prev_ = -1;

      s.next_ = -1;
      s.prev_ = -1;

      ALWAYS_ASSERT(FREE == s.status_);
      s.status_ = WAITING;
      ASSERT(!s.exception_.hasException ());

      ++slots_waiting_;

      return getSlotCB (thisslot);
   }

 
   template <unsigned int C>
   CBType MultiCompletion<C>::getSlotCB (unsigned int pos)
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


   template <unsigned int C>
   MultiCompletion<C>::MultiCompletion ()
      : slots_waiting_(0), slots_completed_(0), first_completed_(-1),
      last_completed_(-1), first_free_(0), slots_exception_(0), waiting_for_(0)
   {
      for (unsigned int i=0; i<C; ++i)
      {
         slots_[i].next_ = i+1;
         slots_[i].prev_ = i-1;
         slots_[i].status_ = FREE;
      }
      slots_[C-1].next_ = -1;
   }

   template <unsigned int C>
   MultiCompletion<C>::~MultiCompletion()
   {
      // Is it up to the user of MultiCompletion to ensure that the object
      // doesn't go out of scope while it is still active or there are
      // exceptions present.
      ALWAYS_ASSERT(0==slots_waiting_
            && "MultiCompletion destroyed with waiting callbacks!");
      ALWAYS_ASSERT(0 == slots_exception_
            && "MultiCompletion destroyed while exceptions are present!");
   }

   template <unsigned int C>
   void MultiCompletion<C>::completeSlot (unsigned int slot, CBException e)
   {
      CBType cb;
      {
         boost::mutex::scoped_lock l (lock_);

#ifndef NDEBUG
         checkInvariants ();
#endif

         Slot & s = slots_[slot];

         ALWAYS_ASSERT(WAITING == s.status_);
         ALWAYS_ASSERT(slots_waiting_);

         ++slots_completed_;
         --slots_waiting_;

         // Add slot to the end of the completed chain
         s.status_ = COMPLETED;
         s.next_ = -1;
         s.prev_ = last_completed_;
         s.exception_.swap (e);

         if (s.exception_.hasException ())
         {
            ++slots_exception_;
         }

         if (s.prev_ != -1)
            slots_[s.prev_].next_ = slot;

         last_completed_ = slot;

         if (first_completed_ < 0)
         {
            ASSERT(-1 == s.next_);
            ASSERT(-1 == s.prev_);
            first_completed_ = slot;
         }

         if  (!waiting_for_)
         {
            // User didn't call wait() yet; we cannot do anything here
            return;
         }

         // The user already called wait ()
         // Check if the condition is fulfilled /or/ if there is an
         // exception condition
         if (waiting_for_ > slots_completed_ && !slots_exception_)
         {
            // Cannot call user callback yet; wait until another slot
            // completes
            return;
         }

         // Make sure nobody else thinks they get to call the callback
         waiting_for_ = 0;
         cb.swap (cb_);

#ifndef NDEBUG
         checkInvariants ();
#endif
      }

      if (cb)
      {
         // If there was an exception, pass a SlotException to the callback
         cb (slots_exception_ ?
            CBException (SlotException ())
          : CBException ());
      }
   }
      
   // Must be called with lock held
   template <unsigned int C>
   void MultiCompletion<C>::removeCompleted (unsigned int pos, CBException & e)
   {

#ifndef NDEBUG
      checkInvariants ();
#endif

      ASSERT(slots_completed_);
      ASSERT(first_completed_ >= 0);

      // Check it has a valid CB status (i.e. not WAITING/FREE)
      Slot & s = slots_[pos];
      ASSERT(s.status_ == COMPLETED);
      e.clear ();
      e.swap (s.exception_);

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
      s.exception_.clear ();

      --slots_completed_;

      // If the slot had an exception, update slots_exception_
      // Use slots_exception_ to see if we need to check in the first place
      if (slots_exception_ && e.hasException ())
      {
         --slots_exception_;
      }

      // If slots_exception_ == 0, e cannot possibly contain an exception
      ASSERT(slots_exception_ || !e.hasException ());

      ASSERT(slots_completed_ || last_completed_ == -1);
      ASSERT(slots_completed_ || first_completed_ == -1);

#ifndef NDEBUG
      checkInvariants ();
#endif
   }

   template <unsigned int C>
   int MultiCompletion<C>::testAny ()
   {
      boost::mutex::scoped_lock l(lock_);

      if (slots_completed_ == 0)
         return -1;

      ASSERT(first_completed_ != -1);

      const unsigned int slot = first_completed_;
      CBException e;
      removeCompleted(slot, e);
      try
      {
         e.check ();
      }
      catch (boost::exception & e)
      {
         // Add the slot number to the exception
         e << slot_number (slot);
         throw;
      }
      catch (...)
      {
         // All exceptions should derive from ZException
         ASSERT(false && "Invalid exception type thrown!");
      }

      return slot;
    }

   template <unsigned int C>
   unsigned int MultiCompletion<C>::testSome (unsigned int * slots, unsigned int max)
   {
      boost::mutex::scoped_lock l(lock_);

      // If an exception slot is present, we always complete that one first.
      if (slots_exception_)
      {
         // find the slot with the exception
         int curslot = first_completed_;
         ASSERT(curslot >= 0);

         // Go through all completed slots until we find the one with the
         // exception
         while (curslot >= 0)
         {
            if (slots_[curslot].exception_.hasException ())
               break;
            curslot = slots_[curslot].next_;
         }

         ALWAYS_ASSERT(curslot >= 0 && slots_[curslot].exception_.hasException ());
         CBException e;
         completeSlot (curslot, e);
         try
         {
            e.check ();
         }
         catch (boost::exception & exception)
         {
            exception << slot_number (curslot);
            throw;
         }
         ALWAYS_ASSERT(false && "e should have an exception!");
         return 1;
      }

      unsigned int count = 0;

      if (!max)
         max = slots_completed_;

      const unsigned int expect = std::min (max, slots_completed_);

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

         CBException e;
         removeCompleted (slot, e);

         // There should never be an exception here
         ALWAYS_ASSERT(!e.hasException ());

         ++count;
      }

      ALWAYS_ASSERT(count == expect);
      return count;
   }

   /**
    * Cancel a wait operation.
    * If cancelled succesfully, calls the wait callback indicating a cancelled
    * operation, and return true after the callback completes.
    *
    * Otherwise, return false. In this case, the wait callback has already
    * been called (or will be/is being called right now). Cancel does not wait
    * until the callback completes in this case.
    */
   template <unsigned int C>
   bool MultiCompletion<C>::cancel ()
   {
      CBType cb;

      {
         boost::mutex::scoped_lock l(lock_);

         if (!waiting_for_)
            return false;

         waiting_for_ = 0;
         cb.swap (cb_);
      }

      cb (CBException::cancelledOperation (0));
      return true;
   }

   template <unsigned int C>
   void MultiCompletion<C>::wait (const CBType & cb, unsigned int count)
   {
      boost::mutex::scoped_lock l(lock_);

      if (!count)
         count = slots_waiting_ + slots_completed_;
      
      // make sure we aren't already waiting
      ALWAYS_ASSERT(!waiting_for_);
      ALWAYS_ASSERT(cb_.empty());

      // If we can complete right away, do it.
      // Unlock before calling callback
      if (count <= slots_completed_ || slots_exception_)
      {
         l.unlock ();
         cb (slots_exception_ ?
               CBException (SlotException ())
             : CBException ());
         return;
      }

      // We'll have to block
      waiting_for_ = count;
      cb_ = cb;
   }

   template <unsigned int S>
   inline unsigned int MultiCompletion<S>::avail () const
   {
      boost::mutex::scoped_lock l(lock_);
      return  S - slots_waiting_ - slots_completed_;
   }

   template <unsigned int S>
   unsigned int MultiCompletion<S>::active () const
   {
      boost::mutex::scoped_lock l (lock_);
      return slots_waiting_ + slots_completed_;
   }

   template <unsigned int S>
   unsigned int MultiCompletion<S>::completed () const
   {
      boost::mutex::scoped_lock l (lock_);
      return slots_completed_;
   }


   template <unsigned int S>
   int MultiCompletion<S>::nextFree () const
   {
      boost::mutex::scoped_lock l (lock_);
      return first_free_;
   }


   template <unsigned int S>
   bool MultiCompletion<S>::isFree (unsigned int pos) const
   {
      boost::mutex::scoped_lock l (lock_);
      return slots_[pos].status_ == FREE;
   }

   template <unsigned int S>
   unsigned int MultiCompletion<S>::size() const
   {
      return S;
   }


   template <unsigned int S>
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
      unsigned int count;

      const unsigned int avail = S - slots_completed_ - slots_waiting_;

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
