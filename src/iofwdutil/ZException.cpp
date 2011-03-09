#include "ZException.hh"
#include "backtrace.hh"

#include <boost/shared_ptr.hpp>
#include <boost/format.hpp>
#include <boost/exception/diagnostic_information.hpp>


using namespace boost;

namespace iofwdutil
{
   //========================================================================

   typedef boost::error_info<struct tag_zexception_user_msg, std::string>
      zexception_usermsg;

  /* ZException::ZException ()
   {
   }

   ZException::ZException (const std::string & s)
   {
      pushMsg (s);
   }

   std::string ZException::toString () const
   {
      return diagnostic_information (*this);
   } */

   /*void ZException::pushMsg (const std::string & msg)
   {
      boost::shared_ptr<std::string> zmsg =
         boost::get_error_info<zexception_msg> (*this);
      if (zmsg)
      {
         *zmsg += msg;
      }
      else
      {
         *this << zexception_msg (msg);
      }
   } */

   std::string getUserErrorMessage (const std::exception & e)
   {
      // Work around get_error_info API modification in
      // certain versions of boost. Sometimes it returns a raw pointer,
      // sometimes a shared pointer.
      if (boost::get_error_info<zexception_usermsg>(e))
         return *boost::get_error_info<zexception_usermsg>(e);
      else
         return std::string ();
   }
   
   boost::exception & addUserErrorMessage (boost::exception & e,
         const std::string & msg)
   {
      // @TODO: add instead of replace
      e << zexception_usermsg (msg);
      return e;
   }

   void addMessage (ZException & e, const std::string & msg)
   {
      e << zexception_msg (msg);
   }

   /**
    * Note: this is not strictly needed. diagnostic_information will try to
    * convert n.value to a string by default, which works in this case since
    * it is already a plain std::string.
    */
   std::string to_string (const zexception_msg & n)
   {
      return str(format("Exception message: %s") % n.value ());
   }

   //========================================================================
}
