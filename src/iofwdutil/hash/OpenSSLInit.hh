#ifndef IOFWDUTIL_HASH_OPENSSLINIT_HH
#define IOFWDUTIL_HASH_OPENSSLINIT_HH

#include "iofwdutil/Singleton.hh"

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      /**
       * This class just makes sure openssl is initialized before we use any
       * of the hash functions.
       */
      class OpenSSLInit : public Singleton<OpenSSLInit>
      {
         public:
            OpenSSLInit ();

      };

      //=====================================================================
   }
}

#endif
