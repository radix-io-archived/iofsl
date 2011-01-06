#ifndef IOFWDEVENT_SINGLECOMPLETION_HH
#define IOFWDEVENT_SINGLECOMPLETION_HH

#include "iofwdevent/Resource.hh"
#include "iofwdutil/assert.hh"

#include "CBType.hh"

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
    * Note: This class is not threadsafe in the sense that only one thread
    * should be calling the test or wait methods.
    *
    * Cannot be copied for two reasons:
    *    1) When using as functor, the user really doesn't want to copy the
    *       functor (and complete the copy) -> force failure is boost::ref
    *       isn't used
    *    2) boost::mutex cannot be copied anyway.
    *
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
      void operator () (CBException e);

   public:
      SingleCompletion ();

      /// Test if the operation already completed.
      /// Can throw
      bool test ();

      /// Wait until the operation completes, or is cancelled.
      /// Can throw.
      void wait ();

      // bool timed_wait ();

      /**
       * Rearm the object so it can be reused for completion testing.
       * Can only be called if the SingleCompletion instance was not yet armed
       * (i.e. no callback reference generated) or it completed.
       */
      void reset ();

      virtual ~SingleCompletion ();

   protected:

      /**
       * Possible states of the SingleCompletion object:
       *   - UNUSED: did not generate callback reference yet
       *   - WAITING: Callback generating, but not yet called
       *   - COMPLETED: callback was called
       */
      enum { UNUSED = 0, WAITING, COMPLETED } status_;

      CBException exception_;

      boost::mutex lock_;
      boost::condition_variable cond_;
   };

//===========================================================================
}

#endif
