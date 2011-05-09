#ifndef IOFWDUTIL_ZEXCEPTION_HH
#define IOFWDUTIL_ZEXCEPTION_HH

#include <boost/exception/exception.hpp>
#include <boost/exception/info.hpp>
#include <boost/exception/error_info.hpp>
#include <boost/exception/get_error_info.hpp>
#include <exception>

#include "iofwd_config.h"

#ifdef EXCEPTION_BACKTRACE
#include "ExceptionBacktrace.hh"
#endif

#include <string>

namespace iofwdutil
{
   //========================================================================

   /**
    * This class should be the base class for all exception classes defined in
    * this project.
    */
   struct ZException : public virtual std::exception,
   public virtual boost::exception  {};

   /*
    * It is possible to add data to an exception object.
    * Define a tag structure, and the type of data you want to associate.
    *
    * For example, the typedef below enables adding a string to an exception
    * object by using   exception_instance << zexception_msg (str)
    *
    * Don't forget to provide a function for converting the tag value into a
    * string if the base value is not default convertible to a string or
    * if the converted value doesn't have a well defined meaning. (For example
    * error codes)
    *
    * To do this: define a to_string (const A &) with A being for example
    * zexception_msg
    */
   typedef boost::error_info<struct tag_zexception_msg,std::string>
      zexception_msg;

   std::string to_string (const zexception_msg & n);

   /**
    * Add a message to the exception object.
    * This should only be used for errors we expect to show to the user!
    */
   boost::exception & addUserErrorMessage (boost::exception & e,
         const std::string & msg);

   /**
    * Retrieve an error message we can show to the user.
    * Return empty string if no such message can be found.
    */
   std::string getUserErrorMessage (const std::exception & e);

   /**
    * Note: defining custom exception types/hierarchies is as easy as:
    *
    * struct MyException : virtual ZException {};
    * struct MoreSpecificMyException : virtual MyException {};
    */

   /**
    * Instead of using throw(x), all code should use ZTHROW(x).
    */

#ifdef EXCEPTION_BACKTRACE
#define ZTHROW(x) BOOST_THROW_EXCEPTION(x \
      << ::iofwdutil::info_exception_backtrace() )
#else
#define ZTHROW(x) BOOST_THROW_EXCEPTION(x)
#endif

   /**
    * Portable boost::get_error_info<T>. Always returns const value_type *
    * ptr.
    */
   template <class ErrorInfo, typename T>
   typename ErrorInfo::error_info::value_type const *
       zexception_info (T const & x)
   {
      // Trick:
      //  older boost versions return shared_ptr<value_type>,
      //  newer versions return const value_type *
      //
      // By using &(*...) we always get a plain pointer.
      return &(*boost::get_error_info<ErrorInfo> (x));
   }

   //========================================================================
}

#endif
