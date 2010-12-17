#ifndef IOFWDUTIL_HASH_HASHEXCEPTION_HH
#define IOFWDUTIL_HASH_HASHEXCEPTION_HH

#include "iofwdutil/ZException.hh"

#include <string>

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      struct HashException : virtual public ZException {};

      struct NoSuchHashException : virtual public HashException {};

      typedef boost::error_info<struct tag_hash_name, std::string> hash_name;


      //=====================================================================
   }
}

#endif
