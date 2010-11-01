#ifndef IOFWDUTIL_HASH_CRC32_HH
#define IOFWDUTIL_HASH_CRC32_HH

#include <boost/cstdint.hpp>
#include "HashFunc.hh"
#include "HashAutoRegister.hh"

namespace iofwdutil
{
   namespace hash
   {
//===========================================================================


      class CRC32 : public HashFunc
      {
         public:
            HASHFUNC_DECLARE(CRC32);

            CRC32 ();

            virtual void reset ();
            virtual std::string getName () const;
            virtual void process (const void * d, size_t bytes);
            virtual size_t getHash (void * dest, size_t bufsize,
                  bool finalize);
            virtual size_t getHashSize () const;

            virtual ~CRC32 ();

        protected:
            boost::uint32_t hash_;
      };

//===========================================================================
   }
}

#endif
