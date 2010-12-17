#ifndef IOFWD_CONFIGEXCEPTION_HH
#define IOFWD_CONFIGEXCEPTION_HH

#include "iofwdutil/ZException.hh"

#include <string>

namespace iofwd
{
   //========================================================================

   /// This exception is a base class for errors caused by incorrect or
   /// missing information in the server's configuration file
   struct ConfigException : public virtual iofwdutil::ZException {};


   /// The key in the configfile
   typedef boost::error_info<struct tag_ce_key, std::string> ce_key;

   typedef boost::error_info<struct tag_ce_environment, std::string>
      ce_environment;

   //========================================================================
}

#endif
