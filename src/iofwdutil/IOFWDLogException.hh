#ifndef IOFWDUTIL_IOFWDLOGEXCEPTION_HH
#define IOFWDUTIL_IOFWDLOGEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace iofwdutil
{
   //========================================================================

   struct LogException : virtual iofwdutil::ZException {};

   struct LogLevelException : virtual LogException {};

   typedef boost::error_info<struct tag_loglevel_name,std::string>
      e_loglevel_name;

   //========================================================================
}

#endif
