#ifndef IOFWDUTIL_ZEXCEPTION_HH
#define IOFWDUTIL_ZEXCEPTION_HH

#include <boost/exception/all.hpp>
#include <exception>

#include <vector>
#include <string>

namespace iofwdutil
{

   /**
    * This class should be the base class for all exception classes defined in
    * this project.
    */
   class ZException : public virtual std::exception,
                      public virtual boost::exception
   {
   public:
      ZException ();

      ZException (const std::string & s);

      void pushMsg (const std::string & msg);

      std::string toString () const;
   };

   typedef boost::error_info<struct tag_zexception_msg,std::string>
      zexception_msg;

   std::string to_string (const zexception_msg & n);


   /**
    * Note: defining custom exception types/hierarchies is as easy as:
    *
    * struct MyException : virtual ZException {};
    * struct MoreSpecificMyException : virtual MyException {};
    */

/**
 * Instead of using throw(x), all code should use ZTHROW(x).
 */
#define ZTHROW(x) BOOST_THROW_EXCEPTION(x)

}

#endif
