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
    */
   typedef boost::error_info<struct tag_zexception_msg,std::string>
      zexception_msg;

   std::string to_string (const zexception_msg & n);

   /**
    * Add a message to the exception object.
    * Can be used to add a human readable description for the user.
    */
   void addMessage (ZException & e, const std::string & msg);

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

   //========================================================================
}

#endif
