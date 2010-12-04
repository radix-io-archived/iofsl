#ifndef IOFWDUTIL_HASH_CRC64_HH
#define IOFWDUTIL_HASH_CRC64_HH

#include "HashFunc.hh"
#include "HashAutoRegister.hh"

namespace iofwdutil
{
   namespace hash
   {
      //======================================================================

      class CRC64 : public HashFunc
      {
         public:
            HASHFUNC_DECLARE(CRC64);

            CRC64 ();

            void reset ();

            virtual std::string getName () const;

            virtual void process (const void * d, size_t size);

            virtual size_t getHash (void * dest, size_t bufsize, bool
                  finalize);

            virtual size_t getHashSize () const;

            uint64_t get () const
            { return state_; }

         protected:

            uint64_t state_;

      };


      //========================================================================
   }
}

#endif
