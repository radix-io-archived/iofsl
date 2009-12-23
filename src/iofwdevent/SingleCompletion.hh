#ifndef IOFWDEVENT_SINGLECOMPLETION_HH
#define IOFWDEVENT_SINGLECOMPLETION_HH

#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/utility.hpp>
#include <csignal>

#include "iofwdevent/Resource.hh"
#include "ResourceOp.hh"
#include "iofwdutil/assert.hh"

namespace iofwdevent
{
//===========================================================================

   /**
    * This class enables a thread to block until a ResourceOp completes.
    * After the operation completes, it can be reused for another operation.
    *
    * Cannot be copied for two reasons:
    *    1) When using as functor, the user really doesn't want to copy the
    *       functor (and complete the copy) -> force failure is boost::ref
    *       isn't use
    *    2) boost::mutex cannot be copied anyway.
    *
    * @TODO: exception transport
    */
   class SingleCompletion : public ResourceOp,
                            public boost::noncopyable
   {
   public:

      /**
       * \brief: CBType compatible functor signature.
       *
       *  Don't forget to use boost::ref !
       */
      void operator () (int status)
      {
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

      virtual void success ();

      virtual void cancel ();

      //virtual void exception ();
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
         status_ = WAITING;
         // Need to reset any stored exception here
      }

      virtual ~SingleCompletion ();

   protected:

      /// Check result, throw if needed
      void checkStatus ();
   protected:

      enum { SUCCESS = 0, CANCEL, EXCEPTION, WAITING };
      sig_atomic_t status_;

      boost::mutex lock_;
      boost::condition_variable cond_;
   };

//===========================================================================
}

#endif
