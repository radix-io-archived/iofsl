#ifndef IOFWDEVENT_CBEXCEPTION_HH
#define IOFWDEVENT_CBEXCEPTION_HH

#include "Handle.hh"

#include <boost/exception_ptr.hpp>

namespace iofwdevent
{
   //========================================================================

   /**
    * This class is used to transport exceptions to the callback.
    * For now, it contains an exception_ptr holding, if any, the exception
    * that occurred during the blocking operation.
    *
    * Instead of just passing the exception pointer, we use this class so that
    * we can easily extend functionality later on without having to go back
    * and change all callbacks.
    *
    * NOTE: This is a value-type and will be passed by value (i.e. copying).
    *
    * @TODO: Add static constructor function for cancelled operation?
    *        (maybe recording the operation handle in the exception?)
    */
   class CBException
   {
      public:
         /// Default constructor: no exception occured
         CBException () {}

         /// Store the specified exception pointer
         CBException (const boost::exception_ptr & ptr)
            : exception_(ptr)
         {
         }

         /**
          * Construct a CBException instance with a pointer to the specified
          * exception. (Note: the exception gets copied). Make sure to pass
          * the full type to this function (i.e. no references/pointers to
          * base classes) or the exception type will get sliced.
          */
         template <typename E>
         inline CBException (const E & exception);

         /**
          * Rethrows the exception (if any).
          */
         inline void check () const;

         /**
          * Rethrow the exception. Before rethrowing, clear it so
          * this->hasException () will return false.
          */
         void checkAndClear ();
         
         /**
          * Returns true if the exception derives from EventCancelledException
          */
         inline bool isCancelled () const;

         /**
          * Return true if there was an exception.
          */
         inline bool hasException () const;

         /**
          * Clear any stored exception.
          */
         inline void clear ();

         /**
          * Return cancelled operation exception.
          * This is just a convenience function. Resources/everyone is free to
          * throw their own exception when cancelling an operation, as long as
          * it derives from iofwdevent::EventCancelledException
          */
         static CBException cancelledOperation (Handle h);


         /**
          * Swap internal state with another CBException
          */
         void swap (CBException & e);

      protected:

         bool checkCancelled () const;
            

      protected:
         boost::exception_ptr exception_;
   };

   //========================================================================

   template <typename E>
   CBException::CBException (const E & except)
      : exception_ (boost::copy_exception (except))
   {
   }

   bool CBException::hasException () const
   {
      return (exception_);
   }

   void CBException::clear ()
   {
      exception_ = boost::exception_ptr();
   }

   void CBException::check () const
   {
      if (0 != exception_)
         boost::rethrow_exception (exception_);
   }

   bool CBException::isCancelled () const
   {
      if (0 == exception_)
         return false;
      return checkCancelled ();
   }

   //========================================================================
}

#endif
