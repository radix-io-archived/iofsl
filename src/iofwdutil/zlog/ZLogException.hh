#ifndef IOFWDUTIL_ZLOG_ZLOGEXCEPTION_HH
#define IOFWDUTIL_ZLOG_ZLOGEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

class ZLogException : public virtual ZException
{
public:
   ZLogException (const std::string & s)
   {
      pushMsg (s);
   }
};

//===========================================================================
   }
}

#endif
