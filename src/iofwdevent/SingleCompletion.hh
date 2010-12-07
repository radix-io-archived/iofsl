#ifndef IOFWDEVENT_SINGLECOMPLETION_HH
#define IOFWDEVENT_SINGLECOMPLETION_HH

#include "iofwdevent/Resource.hh"
#include "iofwdutil/assert.hh"

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>

namespace iofwdevent
{
//===========================================================================

   /**
    * This class enables a thread to block until an operation completes.
    * After the operation completes, it can be reused for another operation.
    *
    * Cannot be copied for two reasons:
    *    1) When using as functor, the user really doesn't want to copy the
    *       functor (and complete the copy) -> force failure is boost::ref
    *       isn't used
    *    2) boost::mutex cannot be copied anyway.
    *
    * @TODO: exception transport
    * @TODO: This class really has a wrong name
    */
   class SingleCompletion : public boost::noncopyable
   {
   public:

      /**
       * THis operator detects when somebody uses us as a callback.
       * In this case, indicate that we're expecting a call and change
       * status to WAITING
       */
      operator CBType ()
      {
         // no lock needed since nobody can call us until we return
         // from this function.
         ALWAYS_ASSERT (status_ == UNUSED);
         status_ = WAITING;
         return CBType(boost::ref(*this));
      }

      /**
       * \brief: CBType compatible functor signature.
       */
      void operator () (int status)
      {
         ALWAYS_ASSERT(status_ == WAITING);
         switch (status)
         {
            case COMPLETED:
               success ();
               break;
            case CANCELLED:
               cancel ();
               break;
            default:
               ALWAYS_ASSERT(false);
         }
      }

   protected:

      void success ();

      void cancel ();

      void exception ();
   public:
      SingleCompletion ();

      /// Test if the operation already completed.
      /// Can throw
      bool test ();

      /// Wait until the operation completes, or is cancelled.
      /// Can throw.
      void wait ();

      // bool timed_wait ();

      /// Rearm the object so it can be reused for completion testing
      void reset ()
      {
         // In principle the lock below is not needed. The user should
         // guarantee that the operation completed before calling reset,
         // so nobody else should be concurrently modifying status_
         boost::mutex::scoped_lock l (lock_);

         ASSERT(status_ != WAITING);
         status_ = UNUSED;
         // Need to reset any stored exception here
      }

      virtual ~SingleCompletion ();

   protected:

      /// Check result, throw if needed
      void checkStatus ();
   protected:

      enum { UNUSED = 0, SUCCESS, CANCEL, EXCEPTION, WAITING };
      int status_;

      boost::mutex lock_;
      boost::condition_variable cond_;
   };

//===========================================================================
}

#endif
