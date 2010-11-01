#include "CRC32.hh"
#include "iofwdutil/assert.hh"
#include "iofwdutil/tools.hh"

extern "C" {
#include <zlib.h>
}

//
// @TODO: Rename this into adler32 and add some wrappers around boost/crc.hpp
// that really implement crc32
//

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================
      
      HASHFUNC_AUTOREGISTER(CRC32, "crc32", 1);

      CRC32::CRC32 ()
      {
      }

      CRC32::~CRC32 ()
      {
         reset ();
      }

      size_t CRC32::getHashSize () const
      {
         return 4;
      }

      std::string CRC32::getName () const
      {
         return "adler32";
      }

      size_t CRC32::getHash (void * dest, size_t bufsize, bool UNUSED(finalize))
      {
         ALWAYS_ASSERT(bufsize >= sizeof(boost::uint32_t));
         * static_cast<boost::uint32_t*>(dest) = hash_;
         return sizeof(boost::uint32_t);
      }

      void CRC32::reset ()
      {
         hash_ = crc32 (0, 0, 0);
      }

      void CRC32::process (const void * d, size_t bytes)
      {
         hash_ = crc32(hash_, (const Bytef*)d, bytes);
      }

      //=====================================================================
   }
}
