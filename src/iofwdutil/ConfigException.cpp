#include <boost/format.hpp>
#include "ConfigException.hh"
#include <boost/exception/all.hpp>

using namespace boost;

namespace iofwdutil
{
   //========================================================================
   
   std::string to_string (const cfexception_key_name & n)
   {
      return str (format ("Config key name: '%s'") % n.value());
   }
   std::string to_string (const cfexception_file_name & n)
   {
      return str (format ("Config filename: '%s'") % n.value());
   }

   //========================================================================
}
