#ifndef SM_SIMPLESLOTS_HH
#define SM_SIMPLESLOTS_HH

#include <boost/bind.hpp>
#include <boost/array.hpp>

#include "iofwdevent/CBType.hh"
#include "iofwdutil/IntrusiveHelper.hh"
#include "SimpleSM.hh"
#include "SMManager.hh"

namespace sm
{
   //========================================================================

   /**
    * This class provides a fixed number of callback slots for
    * a SimpleSM machine.
    *
    * The class directly calls intrusive_ptr_add_ref
    * and intrusive_ptr_release
    *
    * Note: no locking needed, The same callback slot cannot be used
    * concurrently.
    *
    * @TODO: Instead of using a per-slot lock, to reduce class size, consider
    * using a class wide lock. This would only affect cases were multiple
    * slots complete at the same time.
    *
    * @TODO: Consider adding an nextSlot() function that automatically arms
    * the next available slot.
    */
   template <size_t SLOTS, typename T>
   class SimpleSlots
   {
      protected:
         enum SlotStatus { FREE, WAITING, COMPLETED };

         typedef typename T::next_method_t next_method_t;

         typedef SimpleSlots<SLOTS,T> SELF;

      public:
         SimpleSlots (T & client)
            : client_(client)
         {
         }

         ~SimpleSlots ();

         iofwdevent::CBType operator [] (size_t pos);

         /**
          * If the slot already completed, wait sets the next state of T to
          * next. If not, it causes the callback to set T's next state to next
          * when the callback completes.
          *
          * Note that SimpleSlots enforces a deterministic ordering, even if
          * multiple slots are active, as it only allows waiting on a single
          * slot at a time.
          *
          * If the operation completed with an exception, the exception will
          * be passed to the new state.
          *
          * @TODO: Consider adding a next_if_throw parameter and move to that
          * if an exception occured.
          */
         void wait (size_t pos, typename T::next_method_t next);

       protected:

         void callback (int pos, const iofwdevent::CBException e);

         static void callbackhelper (SELF & self, int pos,
               iofwdevent::CBException e);

         template <size_t POS>
         static void callbackT (SELF & self, iofwdevent::CBException e)
         {
            callbackhelper (self, POS, e);
         }

         /**
          * Here we're trading space for speed.
          * Binding more than a pointer worth of data incurs overhead
          * (additional allocation/free, and copying) so instead of binding
          * the slot number and the 'this' pointer, we generate a custom
          * callback function for a slot number, basically encoding the slot
          * number in the address of the function.
          *
          * We do this for the first 10 slots (assuming more than 10 slots
          * will be rarely needed). If a higher slot number is requested,
          * getCallback falls back to binding both parameters.
          */
         iofwdevent::CBType getCallback (size_t pos);

      protected:

         struct Slot
         {
            Slot ()
               : status_(FREE), next_(0)
            {
            }

            SlotStatus status_;
            next_method_t next_;
            iofwdevent::CBException exception_;
         };

         T & client_;

         boost::array<Slot, SLOTS> slots_;
         boost::array<boost::mutex, SLOTS> locks_;
   };

   //========================================================================

   template <size_t SLOTS, typename T>
   SimpleSlots<SLOTS,T>::~SimpleSlots ()
   {
      // Whoever is using SimpleSlots is responsible for cancelling all
      // pending operations before SimpleSlots goes out of scope.

      for (size_t i=0; i<SLOTS; ++i)
      {
         ALWAYS_ASSERT(slots_[i].status_ == FREE);
         ALWAYS_ASSERT(!slots_[i].exception_.hasException ());
      }
   }

   template <size_t SLOTS, typename T>
   iofwdevent::CBType SimpleSlots<SLOTS,T>::operator [] (size_t pos)
   {
      ASSERT(pos < SLOTS);
      Slot & s = slots_[pos];
      ASSERT(s.status_ == FREE);
      s.status_ = WAITING;
      s.next_ = 0;

      // If the status is FREE, there shouldn't be any exception here
      ALWAYS_ASSERT(!s.exception_.hasException ());

      // inc refcount on client to ensure it stays alive until the
      // callback completed.
      //
      // The resource/blocking operation doesn't have a direct reference T, so
      // we need to keep T alive.
      intrusive_ptr_add_ref(&client_);

      return getCallback (pos);
   }

   template <size_t SLOTS, typename T>
   void SimpleSlots<SLOTS,T>::wait (size_t pos, typename T::next_method_t next)
   {
      boost::mutex::scoped_lock l(locks_[pos]);

      ASSERT (pos < SLOTS);
      Slot & s = slots_[pos];
      ALWAYS_ASSERT(s.next_ == 0);

      ASSERT ((s.status_ != FREE) && "Need to arm slot first!");

      if (s.status_ == WAITING)
      {
         // In this case the callback has not occurred yet;
         // The object is kept alive until the callback happens,
         // at which point we will reschedule.
         s.next_ = next;
         return;
      }

      // it already completed (callback was already called)
      // so we don't need to reschedule and can do an internal
      // transition.
      s.status_ = FREE;
      iofwdevent::CBException e;
      e.swap (s.exception_);

      ALWAYS_ASSERT(next);
      client_.setNextMethod (next, e);
   }

   template <size_t SLOTS, typename T>
   iofwdevent::CBType SimpleSlots<SLOTS,T>::getCallback (size_t pos)
   {
      ASSERT( pos < SLOTS);
      switch (pos)
      {
         case 0: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<0>, boost::ref(*this), _1);
         case 1: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<1>, boost::ref(*this), _1);
         case 2: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<2>, boost::ref(*this), _1);
         case 3: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<3>, boost::ref(*this), _1);
         case 4: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<4>, boost::ref(*this), _1);
         case 5: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<5>, boost::ref(*this), _1);
         case 6: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<6>, boost::ref(*this), _1);
         case 7: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<7>, boost::ref(*this), _1);
         case 8: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<8>, boost::ref(*this), _1);
         case 9: return boost::bind(&SimpleSlots<SLOTS,T>::template callbackT<9>, boost::ref(*this), _1);
         default:
                 return boost::bind (&SimpleSlots<SLOTS,T>::callbackhelper,
                       boost::ref(*this), pos, _1);
      }
   }


   template <size_t SLOTS, typename T>
   void SimpleSlots<SLOTS,T>::callback (int pos,
         const iofwdevent::CBException e)
   {
      boost::mutex::scoped_lock l(locks_[pos]);

      Slot & s = slots_[pos];

      ASSERT((s.status_ == WAITING) && "Need to use it as callback"
            " first!");

      if (s.next_)
      {
         // if next was set, we had already executed a 'wait' on this
         // slot. We can transition right away and the slot becomes free.
         // We need to any slot exception as the current pending exception.

         next_method_t next = s.next_;
         s.next_ = 0;
         s.status_ = FREE;

         // We unlock the mutex so, that if we end up destroying
         // ourselves,
         // we don't destroy a locked mutex.
         l.unlock ();

         client_.resumeSM (next, e);
      }
      else
      {
         // Didn't do a wait on this slot yet. Store any exception in the
         // slot and update the status to COMPLETED
         s.status_ = COMPLETED;
         s.exception_ = e;
      }
   }

   
   template <size_t SLOTS, typename T>
   void SimpleSlots<SLOTS,T>::callbackhelper (SELF & self, int pos,
               iofwdevent::CBException e)
   {
      self.callback (pos, e);
      // The callback has happened, we don't care if the client doesn't
      // stay alive.


      // We can end up destroying ourselves if:
      //   -> the client is rescheduled (s.next_) = true
      //   -> it runs before we get to ptr_release
      //   -> it has no more pending actions (i.e. the SimpleSM is dead)

      // As long as somebody is calling our execute method, it should be
      // impossible here to destroy the client since whoever is calling
      // its execute method should still hold a shared pointer to it.

      // to avoid destroying ourselves, consider making callback status
      // and binding ref to *this
      intrusive_ptr_release (&self.client_);
   }


   //========================================================================
}

#endif
