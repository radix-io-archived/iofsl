#ifndef IOFWDUTIL_ZLOG_ZLOGEXCEPTION_HH
#define IOFWDUTIL_ZLOG_ZLOGEXCEPTION_HH

#include "iofwdutil/ZException.hh"

#include <string>

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

struct ZLogException : public virtual ZException {};


struct ZLogUnknownOptionException : public virtual ZLogException {};

struct ZLogInvalidLevelException : public virtual ZLogException {};

struct ZLogParseException : public virtual ZLogException {};

typedef boost::error_info<struct tag_zlog_option_name, std::string>
     zlog_option_name;

typedef boost::error_info<struct tag_zlog_level, unsigned int>
     zlog_level;

/// The problematic text string
typedef boost::error_info<struct tag_zlog_text, std::string>
     zlog_text;

typedef boost::error_info<struct tag_zlog_location, std::string>
   zlog_location;
//===========================================================================
   }
}

#endif
