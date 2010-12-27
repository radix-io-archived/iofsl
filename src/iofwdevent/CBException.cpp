#include "CBException.hh"
#include "EventExceptions.hh"
#include "iofwdutil/assert.hh"

#include <algorithm>
#include <boost/utility.hpp>

namespace iofwdevent
{
   //========================================================================

   CBException CBException::current_exception ()
   {
      return CBException (boost::current_exception ());
   }

   void CBException::swap (CBException & other)
   {
      if (this == boost::addressof(other))
         return;

      // no swap for boost::exception_ptr (but there is for std::exception_ptr
      // it seems)

      // No need for try/catch, if making the copy in the line below fails we
      // didn't modify any state yet, so we're exception clean. The other two
      // statements shouldn't throw.
      boost::exception_ptr tmp = exception_;
      exception_ = other.exception_;
      other.exception_ = tmp;
   }

   CBException CBException::cancelledOperation (Handle h)
   {
      return CBException (EventCancelledException ()
                            << cancelled_handle (h));
   }

   void CBException::checkAndClear ()
   {
      if (!exception_)
         return;

      boost::exception_ptr tmp;
      std::swap (exception_, tmp);
      boost::rethrow_exception (tmp);
   }

   /**
    * Check if the stored exception derives from
    * iofwdevent::EventCancelledException
    */
   bool CBException::checkCancelled () const
   {
      try
      {
         boost::rethrow_exception (exception_);
      }
      catch (const EventCancelledException & e)
      {
         return true;
      }
      catch (...)
      {
         return false;
      }
      // should not get here
      ASSERT(false);
      return false;
   }

   //========================================================================
}
