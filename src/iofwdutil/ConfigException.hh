#ifndef IOFWDUTIL_CONFIGEXCEPTION_HH
#define IOFWDUTIL_CONFIGEXCEPTION_HH

#include "ZException.hh"

namespace iofwdutil
{

   struct ConfigException : virtual public ZException { };


   struct ConfigIOException : virtual public ConfigException { };

   struct CFKeyMissingException : virtual public ConfigException { };

   struct CFKeyValueException : virtual public ConfigException { };

   // Name of the missing key
   typedef boost::error_info<struct tag_cfkey_name,std::string>
      cfexception_key_name;

   typedef boost::error_info<struct tag_cf_filename,std::string>
      cfexception_file_name;

   std::string to_string (const cfexception_key_name & n);
   std::string to_string (const cfexception_file_name & n);

}

#endif
