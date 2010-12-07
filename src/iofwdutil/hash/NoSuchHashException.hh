#ifndef IOFWDUTIL_HASH_NOSUCHHASHEXCEPTION_HH
#define IOFWDUTIL_HASH_NOSUCHHASHEXCEPTION_HH

#include <string>
#include "iofwdutil/ZException.hh"

namespace iofwdutil
{
   namespace hash
   {

      class NoSuchHashException : public ZException
      {
         public:
            NoSuchHashException (const std::string & hashname);

      };

   }
}

#endif
