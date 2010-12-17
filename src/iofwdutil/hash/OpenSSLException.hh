#ifndef IOFWDUTIL_HASH_OPENSSLEXCEPTION_HH
#define IOFWDUTIL_HASH_OPENSSLEXCEPTION_HH

#include "HashException.hh"

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      struct HashOpenSSLException : public virtual HashException {};

      typedef boost::error_info<struct tag_hash_openssl_error,int>
            hash_openssl_error;

      //=====================================================================
   }
}
#endif
