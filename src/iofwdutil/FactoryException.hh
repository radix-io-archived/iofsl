#ifndef IOFWDUTIL_FACTORYEXCEPTION_HH
#define IOFWDUTIL_FACTORYEXCEPTION_HH

#include "ZException.hh"

namespace iofwdutil
{
   struct FactoryException : public virtual ZException { };

   struct NoSuchFactoryKeyException : public virtual FactoryException {};

}

#endif
