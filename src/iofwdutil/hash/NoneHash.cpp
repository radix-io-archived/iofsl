#include "NoneHash.hh"

namespace iofwdutil
{
   namespace hash
   {
      //=====================================================================

      HASHFUNC_AUTOREGISTER(NoneHash, "none", 0);

      //=====================================================================

      NoneHash::NoneHash ()
      {
      }

      NoneHash::~NoneHash ()
      {
      }

      void NoneHash::reset ()
      {
      }

      std::string NoneHash::getName () const
      {
         return "none";
      }

      size_t NoneHash::getHash (void *, size_t, bool )
      {
         return 0;
      }

      size_t NoneHash::getHashSize () const
      {
         return 0;
      }

      void NoneHash::process (const void * , size_t )
      {
      }

      //=====================================================================
   }
}
