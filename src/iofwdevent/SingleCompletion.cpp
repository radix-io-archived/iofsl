#include "iofwdutil/assert.hh"
#include "SingleCompletion.hh"

namespace iofwdevent
{

   void SingleCompletion::success ()
   {
      ALWAYS_ASSERT(status_ == WAITING);
      status_ = SUCCESS;
      cond_.notify_all ();
   }

   void SingleCompletion::cancel ()
   {
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

      checkStatus ();
      return true;
   }

   void SingleCompletion::wait ()
   {
      boost::unique_lock<boost::mutex> l (lock_);

      while (status_ == WAITING)
         cond_.wait (l);

      checkStatus ();
   }

}

