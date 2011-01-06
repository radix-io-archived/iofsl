#include "iofwdutil/assert.hh"
#include "SingleCompletion.hh"

namespace iofwdevent
{
   //========================================================================

   SingleCompletion::SingleCompletion ()
      : status_(UNUSED)
   {
   }

   void SingleCompletion::operator () (CBException e)
   {
      boost::mutex::scoped_lock l (lock_);

      ALWAYS_ASSERT(status_ == WAITING);

      status_ = COMPLETED;
      exception_ = e;

      cond_.notify_all ();
   }

   SingleCompletion::~SingleCompletion ()
   {
      // If status is WAITING, the resource is still going to call our methods
      // even after we're gone. Bad thing.
      //
      // The correct way to deal with this is to cancel the operations and
      // only leave the scope when the operation was properly cancelled.
      ALWAYS_ASSERT(status_ != WAITING);
   }

   bool SingleCompletion::test ()
   {
      boost::mutex::scoped_lock l (lock_);

      ALWAYS_ASSERT(status_ != UNUSED);

      if (status_ == WAITING)
         return false;

      // Rethrow if an exception occurred
      exception_.check ();

      return true;
   }

   void SingleCompletion::wait ()
   {
      ALWAYS_ASSERT(status_ != UNUSED);

      boost::mutex::scoped_lock l (lock_);

      while (status_ == WAITING)
      {
         cond_.wait (l);
      }

      // Call checkstatus here to throw exception if needed
      exception_.check ();
   }

   void SingleCompletion::reset ()
   {
      // In principle the lock below is not needed. The user should
      // guarantee that the operation completed before calling reset,
      // so nobody else should be concurrently modifying status_
      // boost::mutex::scoped_lock l (lock_);

      if (status_ == UNUSED)
         return;

      // it is illegal to reset in WAITING state
      ASSERT(status_ == COMPLETED);

      status_ = UNUSED;
      // Reset any stored
      exception_.clear ();
   }

   //========================================================================
}

