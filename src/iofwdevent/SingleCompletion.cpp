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
      {
         boost::unique_lock<boost::mutex> l (lock_);
         ALWAYS_ASSERT(status_ == WAITING);
         status_ = SUCCESS;
      }
      cond_.notify_all ();
   }

   void SingleCompletion::cancel ()
   {
      {
         boost::unique_lock<boost::mutex> l (lock_);
         ALWAYS_ASSERT(status_ == WAITING);
         status_ = CANCEL;
      }
      cond_.notify_all ();
   }

   SingleCompletion::~SingleCompletion ()
   {
      // If status is WAITING, the resource is still going to call our methods
      // even after we're gone. Bad thing.
      ALWAYS_ASSERT(status_ != WAITING);
   }

   void SingleCompletion::checkStatus ()
   {
      if (status_ != EXCEPTION)
         return;

      ALWAYS_ASSERT(false && "todo");
   }

   bool SingleCompletion::test ()
   {
      if (status_ == WAITING)
         return false;

      // Call checkstatus here to throw exception if needed
      checkStatus ();
      return true;
   }

   void SingleCompletion::wait ()
   {
      boost::unique_lock<boost::mutex> l (lock_);

      while (status_ == WAITING)
         cond_.wait (l);

      // Call checkstatus here to throw exception if needed
      checkStatus ();
   }

}

