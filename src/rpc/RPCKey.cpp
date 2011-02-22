#include <string.h>

#include "iofwdutil/assert.hh"
#include "iofwdutil/hash/CRC64.hh"

#include "RPCKey.hh"

namespace rpc
{
   //========================================================================

   RPCKey getRPCKey (const char * s)
   {
      iofwdutil::hash::CRC64 crc;
      ALWAYS_ASSERT(s);
      crc.process (s, strlen(s));
      STATIC_ASSERT(sizeof(RPCKey) == sizeof(uint64_t));
      uint64_t key;
      crc.getHash (&key, sizeof(key), true);
      return key;
   }

   RPCKey getRPCKey (const std::string & s)
   {
      return getRPCKey (s.c_str());
   }

   //========================================================================
}
