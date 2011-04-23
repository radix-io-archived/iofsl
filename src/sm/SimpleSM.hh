#ifndef SM_SIMPLESM_HH
#define SM_SIMPLESM_HH

#include <boost/thread.hpp>

#include "SMManager.hh"
#include "SMClient.hh"
#include "iofwdevent/CBType.hh"

namespace sm
{
//===========================================================================

   /**
    *
    *   Type T must contain a init(CBException) method.
    *
    */
   template <typename T>
   class SimpleSM : public SMClient
   {
   public:

      typedef void (T::*next_method_t) (iofwdevent::CBException e);


   protected:

      /**
       * This method can be called to yield the CPU;
       * The SimpleSM will be executed once all the other waiting state
       * machines had a chance to execute.
       * (It works be returning true from the execute() method, indicating it
       * should be rerun/rescheduled at some point.
       */
      void yield ()
      { yield_ = true; }

   public:
      SimpleSM (SMManager & smm, bool poll);

      virtual ~SimpleSM ();

      virtual bool execute ();

      template <next_method_t PTR>
      void setNextMethodT (iofwdevent::CBException status)
      {
         setNextMethod (PTR, status);
      }

      /**
       * This method will wake up the SimpleSM if needed, and make it
       * resume execution at the specified method.
       */
      void resumeSM (next_method_t n, iofwdevent::CBException status =
            iofwdevent::CBException ())
      {
         {
           boost::mutex::scoped_lock l(state_lock_);
           // assert needs to be inside lock to prevent store/load race.
           ALWAYS_ASSERT(!next_);

           setNextMethod (n, status);
         }

         if (!running_)
         {
             /* if we are in poll mode, execute the next state now ! */
             //if(poll_)
             //{
             //   execute();
             //}
             ///* other wise, submit to the SMManager */
             //else
             //{
                smm_.schedule(this);
             //}
         }
      }


      /**
       * This method is meant for use from within the SimpleSM to make
       * it go to another state unconditionally and without sleeping.
       */
      void setNextMethod (next_method_t n, iofwdevent::CBException status =
            iofwdevent::CBException ())
      {
         ASSERT(n);
         next_ = n;
         next_status_ = status;
      }


      bool isRunning ()
      {
            boost::mutex::scoped_lock l(state_lock_);
            return running_;
      }

   protected:

      bool unprotected_execute();

      // For rescheduling in setNextMethod
      SMManager & smm_;

      next_method_t next_;
      iofwdevent::CBException next_status_;
      bool yield_;
      bool running_;
      bool poll_;

      boost::mutex state_lock_;
   };

//===========================================================================
//===========================================================================
//===========================================================================

template <typename T>
SimpleSM<T>::SimpleSM (SMManager & m, bool poll = false)
  : smm_(m), next_(0), yield_(false),
   running_(false), poll_(poll)
{
   setNextMethodT<&T::init> (iofwdevent::CBException ());
}

template <typename T>
SimpleSM<T>::~SimpleSM ()
{
   {
   boost::mutex::scoped_lock l(state_lock_);
    ALWAYS_ASSERT(!running_);
    // This might be better as: ZLOG_WARN_IF(!done_called_, "...")
    //ALWAYS_ASSERT(done_called_);
   }
}

template <typename T>
bool SimpleSM<T>::execute ()
{
   boost::intrusive_ptr<sm::SimpleSM<T> > self (this);
   {
      boost::mutex::scoped_lock l2(state_lock_);

      // This shouldn't execute if there isn't a next state to go to.
      ALWAYS_ASSERT(next_);
      ALWAYS_ASSERT(!running_);
      running_ = true;
   }

   do
   {

      iofwdevent::CBException next_status;
      next_method_t next;

      {
         boost::mutex::scoped_lock l2(state_lock_);
         next = next_;
         next_status.swap (next_status_);
         // Need to clear this here because the method we're calling might call
         // setNextMethod
         next_ = 0;
         next_status_.clear ();
      }

      (dynamic_cast<T*>(this)->*next)(next_status);

      {
         boost::mutex::scoped_lock l2(state_lock_);
         if (!next_ || yield_)
         {
            running_ = false;
            // If we yield, we must have something to execute when we get the
            // CPU back. If not, shouldn't call yield.
            ALWAYS_ASSERT (!yield_ || next_);
            break;
         }
      }
   } while (true);

   ALWAYS_ASSERT(alive());
   return yield_;
}

template <typename T>
bool SimpleSM<T>::unprotected_execute()
{
   {
      // This shouldn't execute if there isn't a next state to go to.
      ALWAYS_ASSERT(next_);
      running_ = true;
   }

   do
   {

      iofwdevent::CBException next_status;
      next_method_t next;

      {
         next = next_;
         next_status.swap (next_status_);
         // Need to clear this here because the method we're calling might call
         // setNextMethod
         next_ = 0;
         next_status_.clear ();
      }

      (dynamic_cast<T*>(this)->*next)(next_status);

      {
         if (!next_ || yield_)
         {
            running_ = false;
            // If we yield, we must have something to execute when we get the
            // CPU back. If not, shouldn't call yield.
            ALWAYS_ASSERT (!yield_ || next_);
            break;
         }
      }
   } while (true);

   ALWAYS_ASSERT(alive());
   return yield_;
}


//===========================================================================
}

#endif
