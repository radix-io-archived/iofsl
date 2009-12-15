#include "iofwdutil/assert.hh"
#include "SingleCompletion.hh"

namespace iofwdevent
{

   SingleCompletion::SingleCompletion ()
      : status_(WAITING)
   {
   }

   void SingleCompletion::success ()
   {
      boost::mutex::scoped_lock l (lock_);
      ALWAYS_ASSERT(status_ == WAITING);
      status_ = SUCCESS;

      // Normally I would move this outside of the lock but valgrind
      // doesn't like it.
      cond_.notify_all ();
   }

   void SingleCompletion::cancel ()
   {
      boost::mutex::scoped_lock l (lock_);

      ALWAYS_ASSERT(status_ == WAITING);
      status_ = CANCEL;

      cond_.notify_all ();
   }

   SingleCompletion::~SingleCompletion ()
   {
      // If status is WAITING, the resource is still going to call our methods
      // even after we're gone. Bad thing.
      ALWAYS_ASSERT(status_ != WAITING);
   }

   /** 
    * Should be called with lock held
    */
   void SingleCompletion::checkStatus ()
   {
      if (status_ != EXCEPTION)
         return;

      // Throw stored exception here
      ALWAYS_ASSERT(false && "todo");
   }

   bool SingleCompletion::test ()
   {
      boost::mutex::scoped_lock l (lock_);

      if (status_ == WAITING)
         return false;

      // Call checkstatus here to throw exception if needed
      checkStatus ();
      return true;
   }

   void SingleCompletion::wait ()
   {
      boost::mutex::scoped_lock l (lock_);

      while (status_ == WAITING)
      {
         cond_.wait (l);
      }

      // Call checkstatus here to throw exception if needed
      checkStatus ();
   }

}

