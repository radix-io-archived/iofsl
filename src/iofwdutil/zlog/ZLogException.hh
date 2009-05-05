#ifndef IOFWDUTIL_ZLOG_ZLOGEXCEPTION_HH
#define IOFWDUTIL_ZLOG_ZLOGEXCEPTION_HH

#include "iofwdutil/ZException.hh"

namespace iofwdutil
{
   namespace zlog
   {
//===========================================================================

class ZLogException : public ZException
{
public:
   ZLogException (const std::string & s)
   {
      pushMsg (s); 
   }

   virtual ~ZLogException ()
   {
   }

}; 

//===========================================================================
   }
}

#endif
