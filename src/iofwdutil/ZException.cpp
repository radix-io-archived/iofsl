#include <boost/foreach.hpp>
#include <boost/exception.hpp>
#include <boost/format.hpp>

#include "ZException.hh"
#include "backtrace.hh"

using namespace boost;

namespace iofwdutil
{
   //========================================================================

   ZException::ZException ()
   {
   }

   ZException::ZException (const std::string & s)
   {
      pushMsg (s);
   }

   std::string ZException::toString () const
   {
      return diagnostic_information (*this);
   }

   void ZException::pushMsg (const std::string & msg)
   {
      std::string * zmsg = const_cast<std::string *>(boost::get_error_info<zexception_msg> (*this)); 
      if (zmsg)
      {
         *zmsg += msg;
      }
      else
      {
         *this << zexception_msg (msg);
      }
   }

   std::string to_string (const zexception_msg & n)
   {
      return str(format("Exception message: %s") % n.value ());
   }

   //========================================================================
}
