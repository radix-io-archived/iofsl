#include "Adler32.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

extern "C" {
#include <zlib.h>
}

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      HASHFUNC_AUTOREGISTER(Adler32, "adler32", 1);

      Adler32::Adler32 ()
      {
      }

      Adler32::~Adler32 ()
      {
         reset ();
      }

      size_t Adler32::getHashSize () const
      {
         return 4;
      }

      std::string Adler32::getName () const
      {
         return "adler32";
      }

      size_t Adler32::getHash (void * dest, size_t bufsize, bool UNUSED(finalize))
      {
         ALWAYS_ASSERT(bufsize >= sizeof(boost::uint32_t));
         * static_cast<boost::uint32_t*>(dest) = hash_;
         return sizeof(boost::uint32_t);
      }

      void Adler32::reset ()
      {
         hash_ = adler32 (0, 0, 0);
      }

      void Adler32::process (const void * d, size_t bytes)
      {
         hash_ = adler32(hash_, (const Bytef*)d, bytes);
      }

      //=====================================================================
   }
}
