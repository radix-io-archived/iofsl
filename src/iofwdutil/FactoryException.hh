#ifndef IOFWDUTIL_FACTORYEXCEPTION_HH
#define IOFWDUTIL_FACTORYEXCEPTION_HH

#include "ZException.hh"

namespace iofwdutil
{
   struct FactoryException : public ZException 
   {
      FactoryException (const std::string & s) : ZException ("Factory")
      {
         pushMsg (s);
      }
   };
}

#endif
