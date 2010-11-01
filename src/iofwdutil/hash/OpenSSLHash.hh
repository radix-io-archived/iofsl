#ifndef IOFWDUTIL_HASH_OPENSSLHASH_HH
#define IOFWDUTIL_HASH_OPENSSLHASH_HH

#include <openssl/evp.h>

#include "iofwdutil/hash/HashFunc.hh"
#include "HashAutoRegister.hh"

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      /**
       * Hash functions through OpenSSL.
       * We don't use EVP_get_hashbyname because that causes all hash
       * functions (including unused ones) to be linked in.
       * Instead we only reference the ones we know we'll be needing.
       */
      class OpenSSLHash : public HashFunc
      {
      public:
         OpenSSLHash (const char * name, const EVP_MD * md);

         virtual void reset ();

         virtual std::string getName () const;

         virtual void process (const void * d, size_t bytes);

         virtual size_t getHash (void * dest, size_t bufsize, bool finalize);

         virtual size_t getHashSize () const;

         virtual ~OpenSSLHash ();


         /**
          * Registers the OpenSSL hash functions with the factory
          */
         static void registerHash ();

      protected:
         inline void check (int ret);

         void error (int ret);

      protected:
         EVP_MD_CTX mdctx_;
         const EVP_MD *md_;

         const char * const name_;

      };

      //=====================================================================

      inline void OpenSSLHash::check (int ret)
      {
         if (1 != ret)
            error (ret);
      }

      //=====================================================================
   }
}

#endif
