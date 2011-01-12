#ifndef IOFWDUTIL_EXCEPTIONBACKTRACE_HH
#define IOFWDUTIL_EXCEPTIONBACKTRACE_HH

#include <string>
#include <boost/exception/error_info.hpp>
#include <boost/exception/info.hpp>
#include <boost/scoped_array.hpp>

namespace iofwdutil
{
   //========================================================================

   struct ExceptionBacktrace
   {
      public:

         // Maximum depth of stack trace
         enum { MAXDEPTH = 36 };

         ExceptionBacktrace ();

         std::string toString () const;

         // Need copy constructor due to scoped_array
         ExceptionBacktrace (const ExceptionBacktrace & other);

         ExceptionBacktrace & operator = (const ExceptionBacktrace & other);

      protected:
         boost::scoped_array<void *> trace_;
         int count_;
   };

   typedef boost::error_info<struct tag_exception_backtrace,
           ExceptionBacktrace> exception_backtrace;

   std::string to_string (const ExceptionBacktrace & e);

   // error_info doesn't take default constructor; We need to pass an
   // ExceptionBacktrace instance
   inline exception_backtrace info_exception_backtrace ()
   { return exception_backtrace (ExceptionBacktrace ()); }

   //========================================================================
}


#endif
