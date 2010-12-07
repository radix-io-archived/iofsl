#ifndef IOFWDUTIL_HASH_NONEHASH_HH
#define IOFWDUTIL_HASH_NONEHASH_HH

#include "iofwdutil/hash/HashFunc.hh"
#include "HashAutoRegister.hh"

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      class NoneHash : public HashFunc
      {
         public:
            HASHFUNC_DECLARE(NoneHash);

            NoneHash ();

            virtual ~NoneHash ();

            virtual void reset ();

            virtual std::string getName () const;

            virtual size_t getHash (void * dest, size_t bufsize,
                  bool finalize);

            virtual size_t getHashSize () const;

            virtual void process (const void * d, size_t bytes);
      };

      //=====================================================================
   }
}
#endif
