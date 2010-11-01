#ifndef IOFWDUTIL_HASH_SHA1SIMPLE_HH
#define IOFWDUTIL_HASH_SHA1SIMPLE_HH

#include <stdint.h>
#include "HashFunc.hh"
#include "HashAutoRegister.hh"

extern "C"
{
#include "c-util/sha1.h"
}

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      /**
       * Reference SHA1 implementation in case we don't have openssl
       */
      class SHA1Simple : public HashFunc
      {
         public:
            HASHFUNC_DECLARE(SHA1Simple);

            SHA1Simple ();

            virtual void reset ();

            virtual size_t getHashSize () const;

            virtual void process (const void * d, size_t bytes);

            virtual size_t getHash (void * dest, size_t bufsize,
                  bool finalize);

            virtual std::string getName () const;

         protected:
            SHA1Context ctx_;

          };

      //=====================================================================
   }
}

#endif
