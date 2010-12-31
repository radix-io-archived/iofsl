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
    */
   template <size_t SLOTS, typename T>
   class SimpleSlots
   {
      protected:
         enum { FREE = iofwdevent::LAST, WAITING };

         typedef typename T::next_method_t next_method_t;

         typedef SimpleSlots<SLOTS,T> SELF;

      public:
         SimpleSlots (T & client)
            : client_(client)
         {
         }

         ~SimpleSlots ()
         {
            for (size_t i=0; i<SLOTS; ++i)
            {
               ALWAYS_ASSERT(slots_[i].status_ == FREE);
            }
         }

         iofwdevent::CBType operator [] (size_t pos)
         {
            ASSERT(pos < SLOTS);
            Slot & s = slots_[pos];
            ASSERT(s.status_ == FREE);
            s.status_ = WAITING;
            s.next_ = 0;

            // inc refcount on client to ensure it stays alive until the
            // callback completed.
            intrusive_ptr_add_ref(&client_);

            return getCallback (pos);
         }

         void wait (size_t pos, typename T::next_method_t next)
         {
            boost::mutex::scoped_lock l(locks_[pos]);

            ASSERT (pos < SLOTS);
            Slot & s = slots_[pos];
            ALWAYS_ASSERT(s.next_ == 0);
            ASSERT (s.status_ != FREE);

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
            int status = s.status_;
            s.status_ = FREE;

            ALWAYS_ASSERT(next);
            client_.setNextMethod (next, status);
         }

      protected:

         void callback (int pos, int status)
         {
            {
               boost::mutex::scoped_lock l(locks_[pos]);

               // make copy
               Slot s = slots_[pos];
               slots_[pos].next_ = 0;
               slots_[pos].status_ = status;

               ASSERT((s.status_ == WAITING) && "Need to use it as callback"
                     " first!");
               s.status_ = status;

               if (s.next_)
               {
                  // if s.next_ is set, we have already executed a 'wait' on this
                  // slot. We can transition right away.

                  slots_[pos].next_ = 0;
                  slots_[pos].status_ = FREE;

                  client_.resumeSM (s.next_, s.status_);
               }

               // We unlock the mutex so, that if we end up destroying
               // ourselves,
               // we don't destroy a locked mutex.

            }
         }

         static void callbackhelper (SELF & self, int pos, int status)
         {
            self.callback (pos, status);
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

         template <size_t POS>
         static void callbackT (SELF & self, int status)
         {
            callbackhelper (self, POS, status);
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
         iofwdevent::CBType getCallback (size_t pos)
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


      protected:

         struct Slot
         {
            Slot ()
               : status_(FREE), next_(0)
            {
            }

            int status_;
            next_method_t next_;
         };

         T & client_;

         boost::array<Slot, SLOTS> slots_;
         boost::array<boost::mutex, SLOTS> locks_;
   };

   //========================================================================
}

#endif
