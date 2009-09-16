#ifndef IOFWD_CONFIGEXCEPTION_HH
#define IOFWD_CONFIGEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace iofwd
{

class ConfigException : public iofwdutil::ZException 
{
   public:
      ConfigException (const std::string & s) 
         : ZException (s)
      {
      }
};

}

#endif
