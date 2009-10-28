#ifndef IOFWDEVENT_SINGLECOMPLETION_HH
#define IOFWDEVENT_SINGLECOMPLETION_HH

#include <boost/thread.hpp>
#include <csignal>
#include "ResourceOp.hh"

namespace iofwdevent
{
//===========================================================================

   /**
    * This class enables a thread to block until a ResourceOp completes.
    * After the operation completes, it can be reused for another operation.
    *
    * @TODO: exception transport
    */
   class SingleCompletion : public ResourceOp
   {
   public:

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

      virtual ~SingleCompletion ();

   protected:

      /// Check result, throw if needed
      void checkStatus ();
   protected:

      enum { SUCCESS = 0, CANCEL, EXCEPTION, WAITING };
      sig_atomic_t completed_;
      sig_atomic_t status_;

      boost::mutex lock_;
      boost::condition_variable cond_;
   };

//===========================================================================
}

#endif